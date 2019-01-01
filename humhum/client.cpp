#include "client.h"

/* both frame-based and note-based melody feature are extracted */
int SMelodyFeatureExtraction(char* filename, float*& pFeaBuf, int &nFeaLen, SNote *&Query, int &nNoteLen) {
	/* 0. pitch extraction */
	SPitchContourExtraction(filename, pFeaBuf, nFeaLen);

	/* 1. five point median filtering */
	SMedianFilter(pFeaBuf, nFeaLen);
	if (nFeaLen < 20) {
		printf("Too short! please try again\n");
		if (NULL != pFeaBuf) {
			delete[] pFeaBuf;
			pFeaBuf = NULL;
		}
		return ERROR_CODE_TOO_SHORT_INPUT;
	}

	/* 2. post-processing the pitch sequence and re-sampling the pitch sequence */
	SProcessQuery(pFeaBuf, nFeaLen);
	if (nFeaLen < 20) {
		printf("Too short! please try again\n");
		if (NULL != pFeaBuf) {
			delete[] pFeaBuf;
			pFeaBuf = NULL;
		}
		return ERROR_CODE_TOO_SHORT_INPUT;
	}

	/* 3. note transcription */
	STranscribeQueryNote(pFeaBuf, nFeaLen, Query, nNoteLen);
	if (nFeaLen < 20) {
		printf("Too short! please try again\n");
		if (NULL != pFeaBuf) {
			delete[] pFeaBuf;
			pFeaBuf = NULL;
		}
		return ERROR_CODE_TOO_SHORT_INPUT;
	}

	return 0;
}

/* search one query */
int SClientTester(char* szModel, char* szModelinfo, char* szWav, char* szOut) {
	int i = 0;

	//0, Load Model
	SModel *SQBHModels = NULL;
	char **szModelInfoStrs = NULL;
	int nModels = 0;

	std::cout << "---------------------Loading Model---------------------" << std::endl;
	/*获得midi的模型并转化成midi的数据结构，每首歌的Pitch,Duration等信息存储在SQBHModels, nModels中*/
	int nTotalModel = SLoadModel(szModel, SQBHModels, nModels);
	if (nTotalModel <= 0) {
		printf("Error on loading model!\n");
		return -1;
	}
	std::cout << "nTotalModel(TotalPhraseNum):" << nTotalModel << endl;
	

	/*获得midi的信息，比如歌曲名字*/
	int nTotalSongs = SReadMelodyDBInfoFile(szModelinfo, szModelInfoStrs);
	if (nTotalModel <= 0) {
		printf("Error on loading modelinfo!\n");
		return -1;
	}

	//1, Feature Extraction
	float *pFeaBuf = NULL;
	int nFeaLen = 0;
	SNote *QueryNotes = NULL;
	int nNoteLen = 0;

	std::cout << "---------------------Client's wav Feature Extraction---------------------" << std::endl;
	/*把用户输入的wav格式歌曲转化成midi的数据结构，信息存储在pFeaBuf, nFeaLen, QueryNotes, nNoteLen*/
	SMelodyFeatureExtraction(szWav, pFeaBuf, nFeaLen, QueryNotes, nNoteLen);

	printf("Client's nNoteLen:%d\n", nNoteLen);
	printf("Client's nFeaLen:%d\n", nFeaLen);


	/*匹配部分*/

	////2, Melody Search
	//NoteBasedResStru *myEMDResStru = new NoteBasedResStru[nTotalModel];
	//SNoteBasedMatch(SQBHModels, nModels, QueryNotes, nNoteLen, myEMDResStru, nFeaLen);

	//FrameBasedResStru *myDTWResStru = new FrameBasedResStru[20];
	//SFrameBasedMatch(SQBHModels, nModels, pFeaBuf, nFeaLen, myEMDResStru, 20, myDTWResStru);

	////3, Finalize and print the result
	//FILE *OutFile = fopen(szOut, "a+t");
	//fprintf(OutFile, "%s ", szWav);
	//for (i = 0; i < 20; i++) {
	//	fprintf(OutFile, "%d: %s, %f; ", myDTWResStru[i].nModelID + 1, szModelInfoStrs[myDTWResStru[i].nModelID], myDTWResStru[i].fScore);
	//	printf("%d: %s, %f\n", myDTWResStru[i].nModelID + 1, szModelInfoStrs[myDTWResStru[i].nModelID], myDTWResStru[i].fScore);
	//}
	//fprintf(OutFile, "\n");
	//fclose(OutFile);

	//for (i = 0; i < nTotalSongs; i++) {
	//	if (NULL != SQBHModels[i].PhrasePos) {
	//		delete[] SQBHModels[i].PhrasePos;
	//		SQBHModels[i].PhrasePos = NULL;
	//	}
	//	if (NULL != SQBHModels[i].sNotes) {
	//		delete[] SQBHModels[i].sNotes;
	//		SQBHModels[i].sNotes = NULL;
	//	}
	//}

	//for (i = 0; i < nTotalSongs; i++) {
	//	if (NULL != szModelInfoStrs[i]) {
	//		delete[] szModelInfoStrs[i];
	//		szModelInfoStrs[i] = NULL;
	//	}
	//}
	//if (NULL != myEMDResStru) {
	//	delete[] myEMDResStru;
	//	myEMDResStru = NULL;
	//}
	//if (NULL != myDTWResStru) {
	//	delete[] myDTWResStru;
	//	myDTWResStru = NULL;
	//}
	//if (NULL != SQBHModels) {
	//	delete[] SQBHModels;
	//	SQBHModels = NULL;
	//}
	//if (NULL != szModelInfoStrs) {
	//	delete[] szModelInfoStrs;
	//	szModelInfoStrs = NULL;
	//}
	//if (NULL != QueryNotes) {
	//	delete[] QueryNotes;
	//	QueryNotes = NULL;
	//}
	//if (NULL != pFeaBuf) {
	//	delete[] pFeaBuf;
	//	pFeaBuf = NULL;
	//}
	//return 0;


	return 0;
}
