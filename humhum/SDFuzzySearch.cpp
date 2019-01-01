#include "SDFuzzySearch.h"
#include "matches.h"

/*  Compare two FrameBasedResStru items according fScore field */
int _compareDTWScore(const FrameBasedResStru& a, const FrameBasedResStru& b) {
	return a.fScore < b.fScore;
}

/*  Compare two NoteBasedResStru items according fScore field */
int _compareEMDRes(const NoteBasedResStru& a, const NoteBasedResStru& b) {
	return a.fScore < b.fScore;
}

int SNoteBasedMatch(const SModel *SQBHModels, int nModels, SNote *Query, int QueryLen,
	NoteBasedResStru *myEMDResStru, int SourceLen)
{
	int i, j;
	int nCount = 0;

	int m_LenQuery = 0;
	feature_t m_FeatureQuery[MAX_EMD_FEASIZE];
	float m_WeightQuery[MAX_EMD_FEASIZE];
	int m_DurQuery = 0;

	/* 预处理音符序列  */
	SQueryNoteProcessing(SourceLen, Query, QueryLen,
		m_FeatureQuery, m_WeightQuery, m_LenQuery, m_DurQuery);

	/* 基于音符序列针对每首歌曲的模型进行匹配 */
	for (i = 0; i < nModels; i++) {
		float fModelMin = (float)MAX_NUMBER;
		int nOffset = -1;

		/* 针对每个模型 */
		for (int k = 0; k < SQBHModels[i].nPhraseNum; k++) {
			int NotesSize = SQBHModels[i].nNoteNum - SQBHModels[i].PhrasePos[k];
			int DestLen = SGetNotesPitchLen(SQBHModels[i].sNotes, SQBHModels[i].PhrasePos[k], NotesSize);

			float MinVal = (float)MAX_NUMBER;
			float Dist = 0.0f;
			float MinScale = 0.0f;
			float ScaleFactor = 0.0f;
			int DstLen;

			/* since we donot know the end point of the query, so try several times to find out the best fitting factor */
			// 由于此时还不清楚终止条件，因此进行多次尝试（迭代）以找出最合适的因子
			for (j = 0; j < EMD_SCALING_TIMES; j++) {
				ScaleFactor = (float)(1 + EMD_SCALING_STEP * j - EMD_SCALING_OFFSET);
				DstLen = (int)((SourceLen*ScaleFactor > DestLen) ? DestLen : SourceLen * ScaleFactor);
				Dist = SModelNoteMatching(DstLen,
					SQBHModels[i].sNotes + SQBHModels[i].PhrasePos[k], NotesSize,
					m_FeatureQuery, m_WeightQuery, m_LenQuery, m_DurQuery);

				if (MinVal > Dist) {
					MinVal = Dist;
					MinScale = ScaleFactor;
				}
			}
			// 保存结果
			myEMDResStru[nCount].nModelID = i;
			myEMDResStru[nCount].nPhraseID = k;
			myEMDResStru[nCount].fScore = MinVal;
			myEMDResStru[nCount].fScale = MinScale;
			nCount++;
		}
	}
	/* 根据相似度对结果进行排序 */
	std::sort(myEMDResStru, myEMDResStru + nCount, _compareEMDRes);
	return true;
}

int SFrameBasedMatch(const SModel *SQBHModels, const int nModels, float *fQueryPitchVector,
	int nQueryPitchVectorLen, NoteBasedResStru *myEMDResStru, int nCandidate, FrameBasedResStru *res)
{
	int i = 0, j = 0, k = 0, m = 0;
	float fModelMinDist = (float)MAX_NUMBER;
	float fDist = 0.0f;
	float fSpan = 0.0f, fCenter = 0.0f, s_1 = 0.0f, s1 = 0.0f, s0 = 0.0f, fSmin = 0.0f;

	float *fDesDataBuf = NULL;
	float *fSrcDataBuf = new float[nQueryPitchVectorLen];
	int nRemoveSilenceCount = nQueryPitchVectorLen;
	int MaxTemplatelen = 0;

	/* only consider the survival candidates for the frame matching for re-ranking the candidates */
	for (int n = 0; n < nCandidate; n++) {
		i = myEMDResStru[n].nModelID;
		j = myEMDResStru[n].nPhraseID;
		float fModelMin = MAX_NUMBER + 0.0f;
		int nOffset = -1;

		fModelMinDist = (float)MAX_NUMBER;
		int NotesSize = SQBHModels[i].nNoteNum - SQBHModels[i].PhrasePos[j];
		int nDesLen = SGetNotesPitchLen(SQBHModels[i].sNotes, SQBHModels[i].PhrasePos[j], NotesSize);

		if (NULL == fDesDataBuf) {
			fDesDataBuf = new float[nDesLen * 2];
			MaxTemplatelen = nDesLen * 2;
		}
		else {
			if (nDesLen > MaxTemplatelen) {
				fDesDataBuf = new float[nDesLen * 2];
				MaxTemplatelen = nDesLen * 2;
			}
		}

		int nFar = 0;

		for (k = 0; k < NotesSize; k++) {
			float fVal = (float)((SQBHModels[i].sNotes[SQBHModels[i].PhrasePos[j] + k].fNoteValue - 69.0f) / 12.0 + log(440.0f) / log(2.0f));
			int NoteDur = (int)(SQBHModels[i].sNotes[SQBHModels[i].PhrasePos[j] + k].fNoteDuration);
			for (m = 0; m < NoteDur / myEMDResStru[n].fScale; m++) {
				fDesDataBuf[nFar + m] = fVal;
			}
			nFar += (int)(NoteDur / myEMDResStru[n].fScale);
		}

		SPreprocessMelodyDB(fDesDataBuf, nDesLen, nRemoveSilenceCount, 1);

		// 在竖直方向上进行二分查找，寻找最合适的对齐参数
		fSpan = 0.4f;
		fCenter = 0.0;
		int dtw_tag = 1;
		while (fSpan > 0.01) {
			for (k = 0; k < nRemoveSilenceCount; k++)
				fSrcDataBuf[k] = fQueryPitchVector[k] + fCenter - fSpan;

			s_1 = SCalcDTWDistance(fSrcDataBuf, nRemoveSilenceCount - 3, fDesDataBuf, nDesLen);

			for (k = 0; k < nRemoveSilenceCount; k++)
				fSrcDataBuf[k] = fSrcDataBuf[k] + fSpan;

			if (dtw_tag == 1)
				s0 = SCalcDTWDistance(fSrcDataBuf, nRemoveSilenceCount - 3, fDesDataBuf, nDesLen);
			else
				s0 = fSmin;

			for (k = 0; k < nRemoveSilenceCount; k++)
				fSrcDataBuf[k] = fSrcDataBuf[k] + fSpan;

			s1 = SCalcDTWDistance(fSrcDataBuf, nRemoveSilenceCount - 3, fDesDataBuf, nDesLen);

			fSmin = Min3(s_1, s0, s1);
			dtw_tag = 0;

			if (s_1 - fSmin < EPSILON3)
				fCenter = fCenter - fSpan;

			if (s1 - fSmin < EPSILON3)
				fCenter = fCenter + fSpan;

			//if (s0 - fSmin < EPSILON3)
			//	fCenter = fCenter;

			fSpan /= 2;
			fDist = fSmin;
			if (fDist < fModelMinDist)
				fModelMinDist = fDist;
		}

		if (fModelMin > fModelMinDist) {
			fModelMin = fModelMinDist;
			nOffset = SQBHModels[i].PhrasePos[j];
		}

		if (fDesDataBuf) {
			delete[] fDesDataBuf;
			fDesDataBuf = NULL;
		}

		// 使用基于音符序列的结果和基于帧序列的结果的线性组合
		res[n].fScore = fModelMin * FUSION_RATIO + myEMDResStru[n].fScore*(1 - FUSION_RATIO);
		res[n].nModelID = i;
		res[n].nOffset = nOffset;
	}

	std::sort(res, res + nCandidate, _compareDTWScore);

	if (fSrcDataBuf) {
		delete[] fSrcDataBuf;
		fSrcDataBuf = NULL;
	}

	return 0;
}