#include "matches.h"

int SGetNotesPitchLen(SNote * sNotes, int nStartNote, int nNotes)
{
	int iTotalPitch = 0;
	for (int i = nStartNote; i < nStartNote + nNotes; i++) {
		iTotalPitch += (int)sNotes[i].fNoteDuration;
	}
	return iTotalPitch;
}

void SNormSongNote(SNote * Notes, int iNoteSize, int iLen)
{
	int i;
	int iDesLen = SGetNotesPitchLen(Notes, 0, iNoteSize);

	int iFirstLen;
	if (iDesLen > iLen) {
		iFirstLen = iLen;
	}
	else {
		iFirstLen = iDesLen;
	}

	float fNoteCount = 0.0f;
	float fSumNotesValue = 0.0f;
	for (i = 0; i < iNoteSize; i++) {
		fNoteCount += Notes[i].fNoteDuration;
		fSumNotesValue += Notes[i].fNoteValue*Notes[i].fNoteDuration;
		if (fNoteCount > iFirstLen)
			break;
	}

	float fMeanNotesValue = fSumNotesValue / fNoteCount;
	for (i = 0; i < iNoteSize; i++)
		Notes[i].fNoteValue = Notes[i].fNoteValue - fMeanNotesValue + NOTE_NORMALIZE_VALUE;
}

void SPreprocessMelodyDB(float * fPitchArray, int & iLen, int SourceLen, float Ratio)
{
	int i, j;

	/*清除没有声音的帧*/
	for (i = j = 0; j < iLen; j++) {
		if (fPitchArray[j] > 2) {
			fPitchArray[i++] = fPitchArray[j];
		}
	}

	if (SourceLen > 0) {
		int iDesLen = (int)(SourceLen*Ratio);
		if (iDesLen < iLen) iLen = iDesLen;
	}
	else {
		iLen = i;
	}

	/*归一化音高数组*/
	float fSum = 0.0f;
	for (j = 0; j<iLen; j++) {
		fSum = fSum + fPitchArray[j];
	}
	float fAvg = fSum / iLen;

	for (j = 0; j<iLen; j++) {
		fPitchArray[j] += -fAvg + PITCH_NORMALIZE_VALUE;
	}
}

inline float Min3(float x, float y, float z)
{
	return std::min(std::min(x, y), z);
}

// 尝试用val更新最小值
template<typename T>
inline void minz(T& ans, const T& val) { if (ans > val) ans = val; }

inline float dist(float x, float y) { return (x-y)*(x-y); }


/* ---- Frame-based ---- */

const int QRY_LEN = 800, TPL_LEN = 1200;
const float FINF = 10000;

float SCalcDTWDistance(float * Qry, int lenQry, float * Lib, int lenLib)
{
	if (lenQry > QRY_LEN) {
		printf("Error: Query Length too large\n");
		return -1;
	}
	if (lenLib > TPL_LEN) {
		printf("Error: Template Length too large\n");
		return -1;
	}

	int r, t, iMin, iMax;
	static float dp[QRY_LEN][TPL_LEN];

	// 使用动态规划计算时间序列的距离度量DTW
	dp[0][0] = dist(Qry[0], Lib[0]);
	for (t = 1; t < lenQry; t++) dp[t][0] = dp[t - 1][0] + dist(Qry[t], Lib[0]);
	for (r = 1; r < lenLib; r++) dp[0][r] = dp[0][r - 1] + dist(Qry[0], Lib[r]);
	for (t = 1; t < lenQry; t++) for (r = 1; r < lenLib; r++) {
		dp[t][r] = Min3(dp[t - 1][r], dp[t][r - 1], dp[t - 1][r - 1]) + dist(Qry[t], Lib[r]);
	}

	/* // SDHumming 做法
	for (t = 0; t < lenQry; t++) {
		// 将匹配长度差控制在两倍以内
		iMin = (t + 1) / 2;
		iMax = lenLib - (lenQry - t - 1) / 2;

		// 超出范围认为是无穷大
		for (r = 0; r < iMin; r++) dp[t][r] = FINF;
		for (r = iMax + 1; r < lenLib; r++) dp[t][r] = FINF;

		// 在范围内
		for (r = iMin; r <= iMax; r++) {
			/*if (t < 2 || r < 2) dp[t][r] = 0;
			else {
				float dis = fabs(Qry[t] - Lib[r]);
				dp[t][r] = Min3(dp[t - 1][r - 1] + dis, dp[t - 1][r - 2] + 3 * dis, dp[t - 2][r - 1] + 3 * dis);
			}
		}
	}*/

	return *std::min_element(dp[lenQry - 1], dp[lenQry - 1] + lenLib);
}


/* ---- Note-based ---- */
