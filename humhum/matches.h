#pragma once
#include "STypes.h"
#include<cstdio>
#include<cstdlib>
#include<cstring>
#include<algorithm>

/* ---- Common ---- */

/*  Return the pitch frames number contained in the note sequence */
int SGetNotesPitchLen(SNote *sNotes, int nStartNote, int nNotes);

/*  For normalizing the note sequence, note shift in vertical direction */
void SNormSongNote(SNote *Notes, int iNoteSize, int iLen);

/*
*  For pre-processing (silence & non-speech frame removal,
*  melody contour normalization) the melody template in DB
*/
void SPreprocessMelodyDB(float * fPitchArray, int & iLen, int SourceLen, float Ratio);

inline float Min3(float x, float y, float z)
{
	return std::min(std::min(x, y), z);
}


/* ---- Frame-based ---- */

/* compute the frame-based similarity of two vector */
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

// 链表?
struct node1_t {
	int i;
	float val;
	node1_t *next;
};

// 十字链表?
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
	node2_t x[SIZE * 2];
	float c[SIZE][SIZE];
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
