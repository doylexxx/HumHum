#pragma once

#include "STypes.h"

/* 使用分段的音符序列衡量两段节奏序列的相似度，较快但不那么准确 */
int SNoteBasedMatch(const SModel *SQBHModels, int nModels, SNote *Query, int QueryLen, 
	NoteBasedResStru *myEMDResStru, int nLen);

/* 使用音高序列衡量两端节奏序列的相似度，较慢但准确 */
int SFrameBasedMatch(const SModel *SQBHModels, const int nModels, float *m_pQueryPitchVector,
	int m_nQueryPitchVectorLen, NoteBasedResStru *NoteBasedResStru, int nCandi, FrameBasedResStru *res);