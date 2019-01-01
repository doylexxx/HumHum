/*
* SModel.cpp  SDHumming Melody Database routines
*
* Copyright (c) 2010 Shanda Interactive Entertainment Limited.
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 3 of the License, or
* any later version. See the COPYRIGHT.txt for more details.
*/

#include "SModel.h"
#include <iostream>
#include <stdlib.h>
/*  load song model database into memory from file */
int SLoadModel(char* ModelFile, SModel *&SQBHModels, int &nModels) { //&是引用，不是取地址

	
	int i, j;
	int id, file_count, NoteSize;

	FILE *fp = fopen(ModelFile, "rb");
	if (fp == NULL) {
		printf("reading model file error\n");
		return ERROR_CODE_LOAD_MODEL_ERROR;
	}

	fread(&file_count, sizeof(int), 1, fp);
	nModels = file_count;
	SQBHModels = new SModel[nModels];
	if (NULL == SQBHModels) {
		return ERROR_CODE_CANNOT_ALLOC_MEM;
	}

	if (file_count > MAX_SONGS_NUMBER || file_count < 1) {
		return ERROR_CODE_LOAD_MODEL_ERROR;
	}

	int nCountTotal = 0;
	for (j = 0; j < file_count; j++) {
		fread(&id, sizeof(int), 1, fp);
		fread(&NoteSize, sizeof(int), 1, fp);
		SQBHModels[j].nNoteNum = NoteSize;
		SQBHModels[j].sNotes = new SNote[SQBHModels[j].nNoteNum];

		for (i = 0; i < SQBHModels[j].nNoteNum; i++) {
			fread(&SQBHModels[j].sNotes[i].fNoteValue, sizeof(float), 1, fp);
			fread(&SQBHModels[j].sNotes[i].fNoteDuration, sizeof(float), 1, fp);
			if (SQBHModels[j].sNotes[i].fNoteDuration > 30)
				SQBHModels[j].sNotes[i].fNoteDuration = 30;
		}

		printf("SQBHModels[%d].nNoteNum :%d \n", j,SQBHModels[j].nNoteNum);

		fread(&SQBHModels[j].nPhraseNum, sizeof(int), 1, fp);//乐句切分信息的个数
		SQBHModels[j].PhrasePos = new int[SQBHModels[j].nPhraseNum + 1];
		SQBHModels[j].PhrasePos[0] = 0;

		for (i = 0; i <= SQBHModels[j].nPhraseNum; i++) {
			if (i > 0)
				fread(&SQBHModels[j].PhrasePos[i], sizeof(int), 1, fp);
			else
				SQBHModels[j].PhrasePos[i] = 0;
		}
		SQBHModels[j].nPhraseNum += 1; //如果歌曲内部没有切分点，就整首歌为一个切分段?
		printf("SQBHModels[%d].nPhraseNum :%d \n", j, SQBHModels[j].nPhraseNum);
		nCountTotal += (SQBHModels[j].nPhraseNum);
	}
	fclose(fp);
	return nCountTotal;
}

