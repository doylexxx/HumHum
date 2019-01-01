#pragma once
#include "STypes.h"
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<algorithm>

/* ---- Common ---- */

/* 返回音符序列中的音高帧的数目 */
int SGetNotesPitchLen(SNote *sNotes, int nStartNote, int nNotes);

/* 归一化音符序列，只在竖直方向上修改 */
void SNormSongNote(SNote *Notes, int iNoteSize, int iLen);

/* 预处理数据库中的节奏模板（去除无声音的部分、波形大小归一化）*/
void SPreprocessMelodyDB(float * fPitchArray, int & iLen, int SourceLen, float Ratio);

inline float Min3(float x, float y, float z)
{
	return std::min(std::min(x, y), z);
}


/* ---- Frame-based ---- */

/* 计算基于帧序列比较相似度 */
float SCalcDTWDistance(float* Qry, int lenQry, float* Lib, int lenLib);


/* ---- Note-based ---- */

/*****************************************************************************/
/* feature_t SHOULD BE MODIFIED BY THE USER TO REFLECT THE FEATURE TYPE      */
struct feature_t {
	float px;
	float py;
} ;
/*****************************************************************************/

struct signature_t {
	int n;                // Number of features in the signature 
	feature_t *Features;  // Pointer to the features vector
	float *Weights;       // Pointer to the weights of the features
} ;

struct flow_t {
	int from;             // Feature number in signature 1 
	int to;               // Feature number in signature 2 
	float amount;         // Amount of flow from "from" to "to" 
};

struct node1_t {
	int i;
	float val;
	node1_t *next;
};

struct node2_t {
	int i, j;
	float val;
	node2_t *nextC, *nextR;
};

float SModelNoteMatching(int FirstNum, SNote *Notes, int NoteSize, feature_t *m_FeatureQuery, 
	float *m_WeightQuery, int m_LenQuery, int m_DurQuery);

float SQueryNoteProcessing(int FirstNum, SNote *Notes, int NoteSize, feature_t *m_FeatureQuery, 
	float *m_WeightQuery, int &m_LenQuery, int &m_DurQuery);

// Earth Mover's Distance (EMD) 计算包装类
class CEmdWrapper {
private:
	static const int SIZE = MAX_SIG_SIZE;

	int n1, n2, width;
	node2_t X[SIZE * 2];
	float C[SIZE][SIZE];
	node2_t *endX, *enterX;
	char isX[SIZE][SIZE];
	node2_t *rows[SIZE], *cols[SIZE];
	double maxW;
	float maxC;

public:
	float emd(signature_t *Signature1, signature_t *Signature2, flow_t *Flow, int *FlowSize);

	float init(signature_t *Signature1, signature_t *Signature2);
	void findBasicVariables(node1_t *U, node1_t *V);
	int isOptimal(node1_t *U, node1_t *V);
	int findLoop(node2_t **Loop);
	void newSol();
	void russel(float *S, float *D);
	void addBasicVariable(int minI, int minJ, float *S, float *D,
		node1_t *PrevUMinI, node1_t *PrevVMinJ,
		node1_t *UHead);
};
