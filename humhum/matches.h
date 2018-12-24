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


/* ---- Frame-based ---- */

/* compute the frame-based similarity of two vector */
float SCalcDTWDistance(float* Qry, int lenQry, float* Lib, int lenLib);


/* ---- Note-based ---- */

