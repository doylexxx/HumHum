#pragma once

#include "STypes.h"

/* ʹ�÷ֶε��������к������ν������е����ƶȣ��Ͽ쵫����ô׼ȷ */
int SNoteBasedMatch(const SModel *SQBHModels, int nModels, SNote *Query, int QueryLen, 
	NoteBasedResStru *myEMDResStru, int nLen);

/* ʹ���������к������˽������е����ƶȣ�������׼ȷ */
int SFrameBasedMatch(const SModel *SQBHModels, const int nModels, float *m_pQueryPitchVector,
	int m_nQueryPitchVectorLen, NoteBasedResStru *NoteBasedResStru, int nCandi, FrameBasedResStru *res);