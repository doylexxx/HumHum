#include <stdio.h>
#include <iostream>
using namespace std;

#include <windows.h>

#include <mmsystem.h>//导入声音头文件
#pragma comment(lib,"winmm.lib")//导入声音头文件库

#define MAX_INQUEU 2
#define BUFSIZE 2205
#define _CRT_SECURE_NO_WARNINGS

static HWAVEIN hwi;//handle指向音频输入
static HWAVEOUT hWaveOut;//指向音频输出
static WAVEFORMATEX waveformat;
static WAVEHDR *pwhi, whis[MAX_INQUEU];
static PWAVEHDR pWaveHdrOut;//指向输出
static char waveBufferRecord[MAX_INQUEU][BUFSIZE];

static double xnow, ynow;
static bool timedisply = true;//是否实时显示
static bool recorddisplay = false;//记录是否显示录音波形
static bool record = false;//记录是否录音
static int bufflag = 0;//标记读取哪个缓冲区
static PBYTE pSaveBuffer, pNewBuffer;//用于保存录音波形
static DWORD dwDataLength;
static DWORD Dataplay;//记录当前播放录音的位置
static BOOL dwRecord;

int ShowMenu()
{
	system("cls");
	cout << "******录音管理系统******" << endl;
	cout << " 1. 开始录音" << endl;
	cout << " 2. 停止录音" << endl;
	cout << " 3. 保存录音" << endl;
	cout << " 0. 退出" << endl;
	cout << "************************" << endl;
	int iChoose = 0;
	while (1)
	{
		cin.clear();
		cin.sync(); //fflush(stdin)

		cout << "请选择: ";
		cin >> iChoose;
		if (!cin.good()) continue;
		if (iChoose < 0 || iChoose > 3) continue;
		break;
	}
	return iChoose;
}


void CALLBACK waveInProc(
	HWAVEIN hwi,
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2)
{
	LPWAVEHDR pwh = (LPWAVEHDR)dwParam1;
	if (uMsg == MM_WIM_DATA)
	{
		pwhi = &whis[bufflag];
		waveInUnprepareHeader(hwi, pwhi, sizeof(WAVEHDR));
		pwhi = &whis[MAX_INQUEU - 1 - bufflag];
		pwhi->dwFlags = 0;
		pwhi->dwLoops = 0;
		waveInPrepareHeader(hwi, pwhi, sizeof(WAVEHDR));
		waveInAddBuffer(hwi, pwhi, sizeof(WAVEHDR));
		static unsigned char prevBuf[BUFSIZE];
		memcpy(prevBuf, waveBufferRecord[bufflag], BUFSIZE);
		bufflag = (bufflag + 1) % MAX_INQUEU;

		if (dwRecord == true)
		{
			pNewBuffer = (PBYTE)realloc(pSaveBuffer, dwDataLength + BUFSIZE);//重新分配空间
			pSaveBuffer = pNewBuffer;
			CopyMemory(pSaveBuffer + dwDataLength, prevBuf, BUFSIZE);
			dwDataLength += BUFSIZE;
			// printf("%s\r\n",pSaveBuffer);
		}
	}
}

void StartRecord()
{
	pSaveBuffer = reinterpret_cast<PBYTE>(malloc(1));
	pSaveBuffer = (PBYTE)realloc(pSaveBuffer, 1);
	dwDataLength = 0;
	dwRecord = true;

	memset(&waveformat, 0, sizeof(WAVEFORMATEX));
	waveformat.wFormatTag = WAVE_FORMAT_PCM;
	waveformat.nChannels = 1;
	waveformat.wBitsPerSample = 8;
	waveformat.nSamplesPerSec = 11025L;
	waveformat.nBlockAlign = 1;
	waveformat.nAvgBytesPerSec = 11025L;
	waveformat.cbSize = 0;
	waveInOpen(&hwi, WAVE_MAPPER, &waveformat, (DWORD_PTR)waveInProc, NULL, CALLBACK_FUNCTION);//打开录音设备
	for (int k = 0; k < MAX_INQUEU; k++)
	{
		pwhi = &whis[k];
		pwhi->dwFlags = 0;
		pwhi->dwLoops = 0;
		pwhi->dwBytesRecorded = 0;
		pwhi->dwBufferLength = BUFSIZE;
		pwhi->lpData = waveBufferRecord[k];
	}
	for (int k = 0; k < (MAX_INQUEU - 1); k++)
	{
		pwhi = &whis[k];
		waveInPrepareHeader(hwi, pwhi, sizeof(WAVEHDR));
		waveInAddBuffer(hwi, pwhi, sizeof(WAVEHDR));
	}
	if (waveInStart(hwi) != MMSYSERR_NOERROR)
	{
		printf("waveInStart error");
	}

}

void StopRecord()
{
	dwRecord = false;
	waveInStop(hwi);

}

void SaveRecord()
{
	if (dwRecord == true)
	{
		StopRecord();
	}

	DWORD NumToWrite = 0;
	DWORD dwNumber = 0;
	CHAR FilePathName[MAX_PATH] = { NULL };
	strcpy_s(FilePathName, "a.wav");
	HANDLE FileHandle =
		CreateFile((LPCWSTR)FilePathName, GENERIC_WRITE, FILE_SHARE_READ, NULL,
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	// memset(m_pWaveHdr.lpData, 0, datasize);

	//dwNumber = FCC("RIFF");
	WriteFile(FileHandle, "RIFF", 4, &NumToWrite, NULL);
	dwNumber = dwDataLength + 18 + 20;
	WriteFile(FileHandle, &dwNumber, 4, &NumToWrite, NULL);
	//dwNumber = FCC("WAVE");

	WriteFile(FileHandle, "WAVE", 4, &NumToWrite, NULL);
	//dwNumber = FCC("fmt ");
	WriteFile(FileHandle, "fmt ", 4, &NumToWrite, NULL);
	dwNumber = 16;

	WriteFile(FileHandle, &dwNumber, 4, &NumToWrite, NULL);

	WriteFile(FileHandle, &waveformat.wFormatTag, sizeof(waveformat.wFormatTag), &NumToWrite, NULL);
	WriteFile(FileHandle, &waveformat.nChannels, sizeof(waveformat.nChannels), &NumToWrite, NULL);
	WriteFile(FileHandle, &waveformat.nSamplesPerSec, sizeof(waveformat.nSamplesPerSec), &NumToWrite, NULL);
	WriteFile(FileHandle, &waveformat.nAvgBytesPerSec, sizeof(waveformat.nAvgBytesPerSec), &NumToWrite, NULL);
	WriteFile(FileHandle, &waveformat.nBlockAlign, sizeof(waveformat.nBlockAlign), &NumToWrite, NULL);
	WriteFile(FileHandle, &waveformat.wBitsPerSample, sizeof(waveformat.wBitsPerSample), &NumToWrite, NULL);
	//dwNumber = FCC("data");
	WriteFile(FileHandle, "data", 4, &NumToWrite, NULL);
	dwNumber = dwDataLength;
	WriteFile(FileHandle, &dwNumber, 4, &NumToWrite, NULL);
	WriteFile(FileHandle, pSaveBuffer, dwDataLength, &NumToWrite, NULL);
	SetEndOfFile(FileHandle);
	CloseHandle(FileHandle);
	FileHandle = INVALID_HANDLE_VALUE; // 收尾关闭句柄

	// waveInReset(hwi); //清空内存块
	// waveInClose(hwi); //关闭录音设备
}
//
//int main()
//{
//	while (1)
//	{
//		//1. 显示主菜单
//		int iChoose = ShowMenu();
//		switch (iChoose)
//		{
//		case 0:
//			return 0;
//			break;
//		case 1: //录音开始
//			StartRecord();
//			system("pause");
//			break;
//		case 2: //停止录音
//			StopRecord();
//			system("pause");
//			break;
//		case 3: //保存录音
//			SaveRecord();
//			system("pause");
//			break;
//		default:
//			break;
//		}
//	}
//	return 0;
//}