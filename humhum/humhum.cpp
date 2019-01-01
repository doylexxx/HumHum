#include "humhum.h"
#include <qstringlistmodel.h>
#include <qtextstream.h>
#include <qfile.h>
#include <iostream>



Humhum::Humhum(QWidget *parent)
	: QMainWindow(parent, Qt::WindowCloseButtonHint)
{
	ui.setupUi(this);
	
	// 显示曲库
	QFile dataFile("Song100.txt");
	if (dataFile.open(QFile::ReadOnly | QIODevice::Text))
	{
		QTextStream data(&dataFile);
		QStringList fonts;
		QString line;
		while (!data.atEnd())
		{
			line = data.readLine();
			line.remove('\n');
			fonts << line;
		}
		QStringListModel *model = new QStringListModel(fonts);
		ui.list_mids->setModel(model);
	}
	connect(ui.button_rec, SIGNAL(clicked()),this,SLOT(on_button_rec_click()));
	connect(ui.button_hum, SIGNAL(clicked()), this, SLOT(on_button_hum_click()));
	setFixedSize(this->width(), this->height());
}

void Humhum::on_button_rec_click()
{
	ui.button_rec->setText("stop");
	ui.button_rec->setDisabled(true);
	
	recordAudio();
}

void Humhum::on_button_hum_click()
{
	
}

void Humhum::recordAudio() {
	outputFile.setFileName("test.raw");
	outputFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
	QAudioFormat format;
	// 设置采样频率;
	format.setSampleRate(8000);
	// 设置通道数;
	format.setChannelCount(1);
	
	format.setSampleSize(16);
	
	format.setCodec("audio/pcm");
	
	format.setByteOrder(QAudioFormat::LittleEndian);
	
	format.setSampleType(QAudioFormat::UnSignedInt);
	QAudioDeviceInfo info = QAudioDeviceInfo::defaultInputDevice();

	audio = new QAudioInput(format, this);
	QTimer::singleShot(10000, this, SLOT(stopRecording()));
	audio->start(&outputFile);
	

}

void Humhum::stopRecording() {
	std::cout << "stop" << endl;
	audio->stop();
	outputFile.close();
	delete audio;
	ui.button_rec->setText("REC");
	ui.button_rec->setDisabled(false);
	addWavHeader("test.raw", "test.wav");
}

qint64 Humhum::addWavHeader(QString catheFileName, QString wavFileName)
{
	
	WAVFILEHEADER WavFileHeader;
	qstrcpy(WavFileHeader.RiffName, "RIFF");
	qstrcpy(WavFileHeader.WavName, "WAVE");
	qstrcpy(WavFileHeader.FmtName, "fmt ");

	WavFileHeader.nFmtLength = 16;  
	WavFileHeader.nAudioFormat = 1; 

	qstrcpy(WavFileHeader.DATANAME, "data");

	WavFileHeader.nBitsPerSample = 16;
	WavFileHeader.nBytesPerSample = 2;
	WavFileHeader.nSampleRate = 8000;
	WavFileHeader.nBytesPerSecond = 16000;
	WavFileHeader.nChannleNumber = 1;

	QFile cacheFile(catheFileName);
	QFile wavFile(wavFileName);

	if (!cacheFile.open(QIODevice::ReadWrite))
	{
		return -1;
	}
	if (!wavFile.open(QIODevice::WriteOnly))
	{
		return -2;
	}

	int nSize = sizeof(WavFileHeader);
	qint64 nFileLen = cacheFile.bytesAvailable();

	WavFileHeader.nRiffLength = nFileLen - 8 + nSize;
	WavFileHeader.nDataLength = nFileLen;

	// 先将wav文件头信息写入，再将音频数据写入;
	wavFile.write((char *)&WavFileHeader, nSize);
	wavFile.write(cacheFile.readAll());

	cacheFile.close();
	wavFile.close();

	return nFileLen;
}