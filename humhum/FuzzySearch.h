#pragma once

#include "STypes.h"

/*
*  Use segmented note sequence as input to measure two melody similarity,
*  which is fast but less accurate
*/
int SNoteBasedMatch(const SModel *SQBHModels, int nModels, SNote *Query, int QueryLen, 
	NoteBasedResStru *myEMDResStru, int nLen);

/*
*  Use pitch sequence as input to measure two melody similarity,
*  which is accurate but slow
*/
int SFrameBasedMatch(const SModel *SQBHModels, const int nModels, float *m_pQueryPitchVector,
	int m_nQueryPitchVectorLen, NoteBasedResStru *NoteBasedResStru, int nCandi, FrameBasedResStru *res);