#include "SModel.h"
#include "SBuildModel.h"
#include "midifile.h"
#include <stdlib.h>
#include <iostream>

#include "SUtil.h"
#include "SMelody.h"
int SMelodyFeatureExtraction(char* filename, float*& pFeaBuf, int &nFeaLen, SNote *&Query, int &nNoteLen);

/* search one query */
int SClientTester(char* szModel, char* szModelinfo, char* szWav, char* szOut);
