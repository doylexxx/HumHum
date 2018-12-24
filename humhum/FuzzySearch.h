#pragma once

#include "STypes.h"

int SNoteBasedMatch(const SModel *SQBHModels, int nModels, SNote *Query, int QueryLen, 
	NoteBasedResStru *myEMDResStru, int nLen);
int SFrameBasedMatch(const SModel *SQBHModels, const int nModels, float *m_pQueryPitchVector,
	int m_nQueryPitchVectorLen, NoteBasedResStru *NoteBasedResStru, int nCandi, FrameBasedResStru *res);