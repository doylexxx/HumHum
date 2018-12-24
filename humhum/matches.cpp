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

float SModelNoteMatching(int iFirstNum, SNote *OriNotes, int NoteSize,
	feature_t *m_FeatureQuery, float *m_WeightQuery, int m_LenQuery, int m_DurQuery) 
{
	static CEmdWrapper m_emdComp;
	feature_t m_pTempFea[MAX_EMD_FEASIZE];
	float m_pTempWeight[MAX_EMD_FEASIZE];

	SNote *Notes = new SNote[NoteSize];
	memcpy(Notes, OriNotes, NoteSize * sizeof(SNote));
	int i, count = 0;

	SNormSongNote(Notes, NoteSize, iFirstNum);

	//Caculate The scale factor to SCALE THE TEMPLATE
	float ScaleFactor;
	ScaleFactor = m_DurQuery / (float)iFirstNum;

	float fSum = 0.0f;
	if (iFirstNum > 20) {
		for (i = 0; i < NoteSize; i++) {
			count++;
			fSum += Notes[i].fNoteDuration;
			if (fSum > iFirstNum)
				break;
		}
	}
	iFirstNum = count;

	//Make sure the Note collected is less than the note size
	if (iFirstNum > NoteSize) iFirstNum = NoteSize;

	//Propose the template
	if (iFirstNum > MAX_EMD_FEASIZE)
		printf(" %d\n", iFirstNum);

	int DurTemplate = SGetNotesPitchLen(Notes, 0, iFirstNum);
	float Far(0), FarCnt(1);
	for (i = 0; i < iFirstNum; i++) {
		m_pTempFea[i].px = Far;
		m_pTempFea[i].py = (Notes[i].fNoteValue - 35) * 7;
		Far += Notes[i].fNoteDuration*ScaleFactor;
		m_pTempWeight[i] = Notes[i].fNoteDuration*FarCnt*ScaleFactor;
		FarCnt *= FORGET_FACTOR;
	}

	if (m_LenQuery >= MAX_EMD_FEASIZE)
		printf("Warning: Query feature length reach max\n");
	if (iFirstNum >= MAX_EMD_FEASIZE)
		printf("Warning: Template feature length reach max\n");

	signature_t s1 = { m_LenQuery, m_FeatureQuery, m_WeightQuery };
	signature_t	s2 = { iFirstNum, m_pTempFea, m_pTempWeight };

	for (int x = 0; x < m_LenQuery; x++) {
		if (m_FeatureQuery[x].py < 60)
			m_FeatureQuery[x].py = 60.0f;
	}

	float       e;
	flow_t      flow[MAX_FLOW_NUM];
	int         flowSize;

	// 计算EMD距离
	e = m_emdComp.emd(&s1, &s2, flow, &flowSize);
	e += fabs(m_LenQuery - iFirstNum + 0.0f);

	delete[] Notes;
	return fabs(e);
}

float SQueryNoteProcessing(int iFirstNum, SNote *OriNotes, int NoteSize,
	feature_t *m_FeatureQuery, float *m_WeightQuery, int &m_LenQuery, int &m_DurQuery)
{
	SNote *Notes = new SNote[NoteSize];
	memcpy(Notes, OriNotes, NoteSize * sizeof(SNote));
	int i, count = 0;

	SNormSongNote(Notes, NoteSize, iFirstNum);

	//Caculate The scale factor to SCALE THE TEMPLATE	
	float fSum = 0.0f;
	if (iFirstNum>20) {
		for (i = 0; i<NoteSize; i++) {
			count++;
			fSum += Notes[i].fNoteDuration;
			if (fSum>iFirstNum)
				break;
		}
	}
	iFirstNum = count;

	//Make sure the Note collected is less than the note size
	if (iFirstNum>NoteSize) iFirstNum = NoteSize;

	m_LenQuery = iFirstNum;
	float fOffset = 0.0f;
	float fOffsetCnt = 1.0f;
	for (i = 0; i<iFirstNum; i++) {
		m_FeatureQuery[i].px = fOffset;
		m_FeatureQuery[i].py = (Notes[i].fNoteValue - 35) * 7;
		fOffset += Notes[i].fNoteDuration;
		m_WeightQuery[i] = Notes[i].fNoteDuration * fOffsetCnt;
		fOffsetCnt *= FORGET_FACTOR;
	}

	m_DurQuery = SGetNotesPitchLen(Notes, 0, iFirstNum);
	delete[] Notes;
	return 0;
}


/* ---- EMD计算 ---- */

float CEmdWrapper::emd(signature_t *Signature1, signature_t *Signature2, flow_t *Flow, int *FlowSize) {
	int itr = 0;
	float totalCost;
	float w;
	node2_t *XP;
	flow_t *FlowP;
	node1_t U[SIZE], V[SIZE];

	w = init(Signature1, Signature2);

	if (n1 > 1 && n2 > 1) {/* IF n1 = 1 OR n2 = 1 THEN WE ARE DONE */
		for (itr = 1; itr < MAX_ITERATIONS; itr++) {
			/* FIND BASIC VARIABLES */
			findBasicVariables(U, V);

			/* CHECK FOR OPTIMALITY */
			if (isOptimal(U, V)) {
				//printf("itr: %d\n",itr);
				break;
			}

			/* IMPROVE SOLUTION */
			newSol();

		}

		if (itr == MAX_ITERATIONS)
			fprintf(stderr, "emd: Maximum number of iterations has been reached (%d)\n",
				MAX_ITERATIONS);
	}

	/* COMPUTE THE TOTAL FLOW */
	totalCost = 0;
	if (Flow != NULL)
		FlowP = Flow;
	for (XP = x; XP < endX; XP++) {
		if (XP == enterX)  /* enterX IS THE EMPTY SLOT */
			continue;
		if (XP->i == Signature1->n || XP->j == Signature2->n)  /* DUMMY FEATURE */
			continue;

		if (XP->val == 0)  /* ZERO FLOW */
			continue;

		totalCost += XP->val * c[XP->i][XP->j];
		if (Flow != NULL) {
			FlowP->from = XP->i;
			FlowP->to = XP->j;
			FlowP->amount = XP->val;
			FlowP++;
		}
	}

	if (Flow != NULL)
		*FlowSize = FlowP - Flow;

	/* RETURN THE NORMALIZED COST == EMD */
	return (float)(totalCost / w);
}

float CEmdWrapper::init(signature_t *Signature1, signature_t *Signature2) {
	int i, j;
	float sSum, dSum, diff;
	feature_t *P1, *P2;
	float S[SIZE], D[SIZE];

	n1 = Signature1->n;
	n2 = Signature2->n;

	if (n1 > MAX_SIG_SIZE || n2 > MAX_SIG_SIZE) {
		fprintf(stderr, "emd: Signature size is limited to %d\n", MAX_SIG_SIZE);
		exit(1);
	}

	maxC = 0;

	float DeltaX, DeltaY;
	for (i = 0, P1 = Signature1->Features; i < n1; i++, P1++) {
		for (j = 0, P2 = Signature2->Features; j < n2; j++, P2++) {
			DeltaX = fabs(P1->px - P2->px);
			DeltaY = fabs(P1->py - P2->py);

			DeltaX = DeltaX>110 ? 110 : DeltaX;
			DeltaY = DeltaY>350 ? 350 : DeltaY;

			int IndexX = (int)((DeltaX + 0.25) / 5);
			int IndexY = (int)((DeltaY + 0.25) / 5);

			int Index = IndexX * 71 + IndexY;
			c[i][j] = (float)sqrt(DeltaX*DeltaX + DeltaY * DeltaY);

			if (c[i][j] > maxC)
				maxC = c[i][j];
		}
	}

	memcpy(S, Signature1->Weights, sizeof(float)*n1);
	memcpy(D, Signature2->Weights, sizeof(float)*n2);

	sSum = 0.0;
	for (i = 0; i < n1; i++) {
		sSum += Signature1->Weights[i];
		rows[i] = NULL;
	}

	dSum = 0.0;
	for (j = 0; j < n2; j++) {
		dSum += Signature2->Weights[j];
		cols[j] = NULL;
	}

	diff = sSum - dSum;
	if (fabs(diff) >= EPSILON * sSum) {
		if (diff < 0.0) {
			memset(c[n1], 0, sizeof(float)*n2);
			S[n1] = -diff;
			rows[n1] = NULL;
			n1++;
		}
		else {
			for (i = 0; i < n1; i++)
				c[i][n2] = 0;
			D[n2] = diff;
			cols[n2] = NULL;
			n2++;
		}
	}

	memset(isX[0], 0, SIZE*SIZE);

	endX = x;

	maxW = sSum > dSum ? sSum : dSum;

	russel(S, D);

	enterX = endX++;  /* AN EMPTY SLOT (ONLY n1+n2-1 BASIC VARIABLES) */

	return sSum > dSum ? dSum : sSum;
}

void CEmdWrapper::findBasicVariables(node1_t *U, node1_t *V) {
	int i, j, found;
	int UfoundNum, VfoundNum;
	node1_t u0Head, u1Head, *CurU, *PrevU;
	node1_t v0Head, v1Head, *CurV, *PrevV;

	/* INITIALIZE THE ROWS LIST (U) AND THE COLUMNS LIST (V) */
	u0Head.next = CurU = U;
	for (i = 0; i < n1; i++) {
		CurU->i = i;
		CurU->next = CurU + 1;
		CurU++;
	}
	(--CurU)->next = NULL;
	u1Head.next = NULL;

	CurV = V + 1;
	v0Head.next = n2 > 1 ? V + 1 : NULL;
	for (j = 1; j < n2; j++) {
		CurV->i = j;
		CurV->next = CurV + 1;
		CurV++;
	}
	(--CurV)->next = NULL;
	v1Head.next = NULL;

	/* THERE ARE n1+n2 VARIABLES BUT ONLY n1+n2-1 INDEPENDENT EQUATIONS,
	SO SET V[0]=0 */
	V[0].i = 0;
	V[0].val = 0;
	v1Head.next = V;
	v1Head.next->next = NULL;

	/* LOOP UNTIL ALL VARIABLES ARE FOUND */
	UfoundNum = VfoundNum = 0;
	while (UfoundNum < n1 || VfoundNum < n2) {
		found = 0;
		if (VfoundNum < n2) {
			/* LOOP OVER ALL MARKED COLUMNS */
			PrevV = &v1Head;
			for (CurV = v1Head.next; CurV != NULL; CurV = CurV->next) {
				j = CurV->i;
				/* FIND THE VARIABLES IN COLUMN j */
				PrevU = &u0Head;
				for (CurU = u0Head.next; CurU != NULL; CurU = CurU->next) {
					i = CurU->i;
					if (isX[i][j]) {
						/* COMPUTE U[i] */
						CurU->val = c[i][j] - CurV->val;
						/* ...AND ADD IT TO THE MARKED LIST */
						PrevU->next = CurU->next;
						CurU->next = u1Head.next != NULL ? u1Head.next : NULL;
						u1Head.next = CurU;
						CurU = PrevU;
					}
					else
						PrevU = CurU;
				}
				PrevV->next = CurV->next;
				VfoundNum++;
				found = 1;
			}
		}
		if (UfoundNum < n1) {
			/* LOOP OVER ALL MARKED ROWS */
			PrevU = &u1Head;
			for (CurU = u1Head.next; CurU != NULL; CurU = CurU->next) {
				i = CurU->i;
				/* FIND THE VARIABLES IN ROWS i */
				PrevV = &v0Head;
				for (CurV = v0Head.next; CurV != NULL; CurV = CurV->next) {
					j = CurV->i;
					if (isX[i][j]) {
						/* COMPUTE V[j] */
						CurV->val = c[i][j] - CurU->val;
						/* ...AND ADD IT TO THE MARKED LIST */
						PrevV->next = CurV->next;
						CurV->next = v1Head.next != NULL ? v1Head.next : NULL;
						v1Head.next = CurV;
						CurV = PrevV;
					}
					else
						PrevV = CurV;
				}
				PrevU->next = CurU->next;
				UfoundNum++;
				found = 1;
			}
		}
		if (!found) {
			fprintf(stderr, "emd: Unexpected error in findBasicVariables!\n");
			fprintf(stderr, "This typically happens when the EPSILON defined in\n");
			fprintf(stderr, "emd.h is not right for the scale of the problem.\n");
			exit(1);
		}
	}
}

void CEmdWrapper::newSol() {
	int i, j, k;
	float xMin;
	int steps;
	node2_t *Loop[2 * SIZE], *CurX, *LeaveX;

	/* ENTER THE NEW BASIC VARIABLE */
	i = enterX->i;
	j = enterX->j;
	isX[i][j] = 1;
	enterX->nextC = rows[i];
	enterX->nextR = cols[j];
	enterX->val = 0;
	rows[i] = enterX;
	cols[j] = enterX;

	/* FIND A CHAIN REACTION */
	steps = findLoop(Loop);

	/* FIND THE LARGEST VALUE IN THE LOOP */
	xMin = EMD_INFINITY;
	for (k = 1; k < steps; k += 2) {
		if (Loop[k]->val < xMin) {
			LeaveX = Loop[k];
			xMin = Loop[k]->val;
		}
	}

	/* UPDATE THE LOOP */
	for (k = 0; k < steps; k += 2) {
		Loop[k]->val += xMin;
		Loop[k + 1]->val -= xMin;
	}

	/* REMOVE THE LEAVING BASIC VARIABLE */
	i = LeaveX->i;
	j = LeaveX->j;
	isX[i][j] = 0;
	if (rows[i] == LeaveX) {
		rows[i] = LeaveX->nextC;
	}
	else {
		for (CurX = rows[i]; CurX != NULL; CurX = CurX->nextC) {
			if (CurX->nextC == LeaveX) {
				CurX->nextC = CurX->nextC->nextC;
				break;
			}
		}
	}

	if (cols[j] == LeaveX)
		cols[j] = LeaveX->nextR;
	else {
		for (CurX = cols[j]; CurX != NULL; CurX = CurX->nextR) {
			if (CurX->nextR == LeaveX) {
				CurX->nextR = CurX->nextR->nextR;
				break;
			}
		}
	}

	/* SET enterX TO BE THE NEW EMPTY SLOT */
	enterX = LeaveX;
}

int CEmdWrapper::findLoop(node2_t **Loop) {
	int i, steps;
	node2_t **CurX, *NewX;
	char IsUsed[2 * SIZE];

	for (i = 0; i < n1 + n2; i++)
		IsUsed[i] = 0;

	CurX = Loop;
	NewX = *CurX = enterX;
	IsUsed[enterX - x] = 1;
	steps = 1;

	do
	{
		if (steps % 2 == 1) {
			/* FIND AN UNUSED X IN THE ROW */
			NewX = rows[NewX->i];
			while (NewX != NULL && IsUsed[NewX - x])
				NewX = NewX->nextC;
		}
		else {
			/* FIND AN UNUSED X IN THE COLUMN, OR THE ENTERING X */
			NewX = cols[NewX->j];
			while (NewX != NULL && IsUsed[NewX - x] && NewX != enterX)
				NewX = NewX->nextR;
			if (NewX == enterX)
				break;
		}

		if (NewX != NULL) {/* FOUND THE NEXT X */
						   /* ADD X TO THE LOOP */
			*++CurX = NewX;
			IsUsed[NewX - x] = 1;
			steps++;
		}
		else {/* DIDN'T FIND THE NEXT X */
			  /* BACKTRACK */
			do
			{
				NewX = *CurX;
				do
				{
					if (steps % 2 == 1)
						NewX = NewX->nextR;
					else
						NewX = NewX->nextC;
				} while (NewX != NULL && IsUsed[NewX - x]);

				if (NewX == NULL) {
					IsUsed[*CurX - x] = 0;
					CurX--;
					steps--;
				}
			} while (NewX == NULL && CurX >= Loop);

			IsUsed[*CurX - x] = 0;
			*CurX = NewX;
			IsUsed[NewX - x] = 1;
		}
	} while (CurX >= Loop);

	if (CurX == Loop) {
		fprintf(stderr, "emd: Unexpected error in findLoop!\n");
		exit(1);
	}

	return steps;
}

void CEmdWrapper::russel(float *S, float *D) {
	int i, j, found, minI, minJ;
	float deltaMin, oldVal, diff;
	float Delta[SIZE][SIZE];
	node1_t Ur[SIZE], Vr[SIZE];
	node1_t uHead, *CurU, *PrevU;
	node1_t vHead, *CurV, *PrevV;
	node1_t *PrevUMinI, *PrevVMinJ, *Remember;

	/* INITIALIZE THE ROWS LIST (Ur), AND THE COLUMNS LIST (Vr) */
	/* 0. Init List, Ur, Vr */
	uHead.next = CurU = Ur;
	for (i = 0; i < n1; i++) {
		CurU->i = i;
		CurU->val = -EMD_INFINITY;
		CurU->next = CurU + 1;
		CurU++;
	}
	(--CurU)->next = NULL;

	vHead.next = CurV = Vr;
	for (j = 0; j < n2; j++) {
		CurV->i = j;
		CurV->val = -EMD_INFINITY;
		CurV->next = CurV + 1;
		CurV++;
	}
	(--CurV)->next = NULL;

	/* 1. FIND THE MAXIMUM ROW AND COLUMN VALUES (Ur[i] AND Vr[j]) */
	float v;
	for (i = 0; i < n1; i++) {
		for (j = 0; j < n2; j++) {
			v = c[i][j];
			if (Ur[i].val <= v)
				Ur[i].val = v;
			if (Vr[j].val <= v)
				Vr[j].val = v;
		}
	}

	/* 2. COMPUTE THE Delta MATRIX */
	for (i = 0; i < n1; i++) {
		for (j = 0; j < n2; j++)
			Delta[i][j] = c[i][j] - Ur[i].val - Vr[j].val;
	}

	/* 2. FIND THE BASIC VARIABLES  */
	do {
		/* FIND THE SMALLEST Delta[i][j] */
		found = 0;
		deltaMin = EMD_INFINITY;
		PrevU = &uHead;
		for (CurU = uHead.next; CurU != NULL; CurU = CurU->next) {
			int i;
			i = CurU->i;
			PrevV = &vHead;
			for (CurV = vHead.next; CurV != NULL; CurV = CurV->next) {
				int j;
				j = CurV->i;
				if (deltaMin > Delta[i][j]) {
					deltaMin = Delta[i][j];
					minI = i;
					minJ = j;
					PrevUMinI = PrevU;
					PrevVMinJ = PrevV;
					found = 1;
				}
				PrevV = CurV;
			}
			PrevU = CurU;
		}

		if (!found)
			break;

		/* ADD X[minI][minJ] TO THE BASIS, AND ADJUST SUPPLIES AND COST */
		Remember = PrevUMinI->next;
		addBasicVariable(minI, minJ, S, D, PrevUMinI, PrevVMinJ, &uHead);

		/* UPDATE THE NECESSARY Delta[][] */
		if (Remember == PrevUMinI->next) {/* LINE minI WAS DELETED */
			for (CurV = vHead.next; CurV != NULL; CurV = CurV->next) {
				int j;
				j = CurV->i;
				if (CurV->val == c[minI][j]) {/* COLUMN j NEEDS UPDATING */
											   /* FIND THE NEW MAXIMUM VALUE IN THE COLUMN */
					oldVal = CurV->val;
					CurV->val = -EMD_INFINITY;
					for (CurU = uHead.next; CurU != NULL; CurU = CurU->next) {
						int i;
						i = CurU->i;
						if (CurV->val <= c[i][j])
							CurV->val = c[i][j];
					}

					/* IF NEEDED, ADJUST THE RELEVANT Delta[*][j] */
					diff = oldVal - CurV->val;
					if (fabs(diff) < EPSILON * maxC)
						for (CurU = uHead.next; CurU != NULL; CurU = CurU->next)
							Delta[CurU->i][j] += diff;
				}
			}
		}
		else {/* COLUMN minJ WAS DELETED */
			for (CurU = uHead.next; CurU != NULL; CurU = CurU->next) {
				int i;
				i = CurU->i;
				if (CurU->val == c[i][minJ]) {/* ROW i NEEDS UPDATING */
											   /* FIND THE NEW MAXIMUM VALUE IN THE ROW */
					oldVal = CurU->val;
					CurU->val = -EMD_INFINITY;
					for (CurV = vHead.next; CurV != NULL; CurV = CurV->next) {
						int j;
						j = CurV->i;
						if (CurU->val <= c[i][j])
							CurU->val = c[i][j];
					}

					/* If NEEDED, ADJUST THE RELEVANT Delta[i][*] */
					diff = oldVal - CurU->val;
					if (fabs(diff) < EPSILON * maxC)
						for (CurV = vHead.next; CurV != NULL; CurV = CurV->next)
							Delta[i][CurV->i] += diff;
				}
			}
		}
	} while (uHead.next != NULL || vHead.next != NULL);
}

void CEmdWrapper::addBasicVariable(int minI, int minJ, float *S, float *D,
	node1_t *PrevUMinI, node1_t *PrevVMinJ, node1_t *UHead)
{
	float T;

	if (fabs(S[minI] - D[minJ]) > EPSILON * maxW)/* DEGENERATE CASE */ {/* DEGENERATE CASE */
		if (S[minI] < D[minJ]) {/* SUPPLY EXHAUSTED */
			T = S[minI];
			S[minI] = 0;
			D[minJ] -= T;
		}
		else {  /* DEMAND EXHAUSTED */
			T = D[minJ];
			D[minJ] = 0;
			S[minI] -= T;
		}
	}
	else {
		T = S[minI];
		S[minI] = 0;
		D[minJ] -= T;
	}

	/* X(minI,minJ) IS A BASIC VARIABLE */
	isX[minI][minJ] = 1;

	endX->val = T;
	endX->i = minI;
	endX->j = minJ;
	endX->nextC = rows[minI];
	endX->nextR = cols[minJ];
	rows[minI] = endX;
	cols[minJ] = endX;
	endX++;

	/* DELETE SUPPLY ROW ONLY IF THE EMPTY, AND IF NOT LAST ROW */
	if (S[minI] == 0 && UHead->next->next != NULL)
		PrevUMinI->next = PrevUMinI->next->next;  /* REMOVE ROW FROM LIST */
	else
		PrevVMinJ->next = PrevVMinJ->next->next;  /* REMOVE COLUMN FROM LIST */
}

int CEmdWrapper::isOptimal(node1_t *U, node1_t *V)
{
	float delta, deltaMin;
	int i, j, minI, minJ;

	/* FIND THE MINIMAL Cij-Ui-Vj OVER ALL i,j */

	float val;
	deltaMin = EMD_INFINITY;
	for (i = 0; i < n1; i++) {
		val = U[i].val;
		for (j = 0; j < n2; j++) {
			if (!isX[i][j]) {
				delta = c[i][j] - val - V[j].val;
				if (deltaMin > delta) {
					deltaMin = delta;
					minI = i;
					minJ = j;
				}
			}
		}
	}

	enterX->i = minI;
	enterX->j = minJ;

	/* IF NO NEGATIVE deltaMin, WE FOUND THE OPTIMAL SOLUTION */
	return deltaMin >= -EPSILON * maxC;
}