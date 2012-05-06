/*******************************************************************
 * Celtic3D.c
 * Generate 3D celtic knot patterns
 *
 * David Llewellyn-Jones
 * http://www.flypig.co.uk
 *
 * July 2011
 *******************************************************************
*/

/* Includes */
#include "celtic.h"
#include "celtic3d.h"
#include "celtic_private.h"

#define _USE_MATH_DEFINES
#include <math.h>

/* Defines */
#define MAX_SIZE (1280)
#define WEAVE_HEIGHT (0.3f)
#define CONTROL_SCALE (0.4f)
#define BEZIERS_PER_TILE (8)
#define SelectRandom(ARRAY, NUM) (ARRAY)[(rand() % (NUM))];
#define COST_PER_UNIT_CUBED (10.0f / (10.0f * 10.0f * 10.0f))

/* Enums */

/* Structures */

/* Function prototypes */

/* Virtual function prototypes */
void DeleteCelticPersist3D (CelticPersist * psCelticData);
void GenerateKnot3D (CelticPersist * psCelticData);
void RenderKnots3D (CelticPersist * psCelticData);

/* Characteristics */
float GetVolume3D (CelticPersist * psCelticData);

/* Settings loading/saving */
void SaveSettingsCeltic3D (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
void LoadSettingsStartCeltic3D (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
void LoadSettingsEndCeltic3D (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
bool ExportModel3D (char const * szFilename, bool boBinary, CelticPersist * psCelticData);

/* Local function prototypes */
static void ImageBezierSplit (Vector3 const * pvStart, Vector3 const * pvStartDir, Vector3 const * pvEnd, Vector3 const * pvEndDir, float fRatio, Vector3 const * pvWeave, Vector3 const * pvOverride, ColFloats const * psStartColour, ColFloats const * psEndColour, RenderPersist * psRenderData);
static Vector3 GetWeaveCorner (VecInt3 const * pvnCorner, RenderPersist * psRenderData);
static Vector3 GetWeaveCentre (VecInt3 const * pvnCorner, RenderPersist * psRenderData);
static void TileLine (Vector3 const * pvPos, Vector3 const * pvSize, VecInt3 const * pvnCornerStart, VecInt3 const * pvnCornerEnd, TILE eStartDir, TILE eEndDir, ColFloats const * psStartColour, ColFloats const * psEndColour, RenderPersist * psRenderData);
static void TileKnot (Vector3 const * pvPos, Vector3 const * pvSize, TILE aeEdge[2][2][2], TILE eCentre, Cube3Colour const * psColours, RenderPersist * psRenderData);
static TILE GetCorner (VecInt3 const * pvnPos, VecInt3 const * pvnCorner, CelticPersist * psCelticData);
static void SetCorner (VecInt3 const * pvnPos, VecInt3 const * pvnCorner, TILE eValue, CelticPersist * psCelticData);
static TILE GetCentre (VecInt3 const * pvnPos, CelticPersist * psCelticData);
static void SetCentre (VecInt3 const * pvnPos, TILE eValue, CelticPersist * psCelticData);
static void PopulateTiles (TILE eOrientation, CelticPersist * psCelticData);
static void SymmetrifyTiles (CelticPersist * psCelticData);
static VecInt3 RotateTile (VecInt3 const * pvnPos, VecInt3 const * pvnSize, VecInt3 const * pvnAxis);
static void FindRotatedTiles (VecInt3 const * pvnPos, VecInt3 const * pvnSize, VecInt3 * anRotatedPos, int nArraySize);
static int ColourTiles (CelticPersist * psCelticData);
static void FollowKnotInside (VecInt3 * pvnPos, VecInt3 * pvnCorner, CelticPersist * psCelticData);
static void FollowKnotToNext (VecInt3 * pvnPos, VecInt3 * pvnCorner, CelticPersist * psCelticData);
static void ColourLoop (VecInt3 * pvnPos, VecInt3 * pvnCorner, Cube3Complete * asCompleted, ColFloats const * psColour, CelticPersist * psCelticData);
static void SelectColour (ColFloats * psColour, RenderPersist * psRenderData);
static void CelticLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
static void CelticLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
static void CelticLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);

/* Function defininitions */
CelticPersist * NewCelticPersist3D (int nWidth, int nHeight, int nDepth, float fTileX, float fTileY, float fTileZ, BezPersist * psBezData) {
	CelticPersist * psCelticData;

	psCelticData = (CelticPersist *)calloc (1, sizeof (CelticPersist));
	psCelticData->uSeed = 0u;
	psCelticData->nLoops = 0;
	psCelticData->vnSize.nX = nWidth;
	psCelticData->vnSize.nY = nHeight;
	psCelticData->vnSize.nZ = nDepth;
	psCelticData->vTileSize.fX = fTileX;
	psCelticData->vTileSize.fY = fTileY;
	psCelticData->vTileSize.fZ = fTileZ;
	psCelticData->fWeirdness = 0.0f;
	psCelticData->eOrientation = TILE_HORIZONTAL;
	psCelticData->boSymmetrify = TRUE;
	psCelticData->boDebug = FALSE;
	psCelticData->psRenderData = NewRenderPersist (-(((float)nWidth) * fTileX / 2.0f), -(((float)nHeight) * fTileY / 2.0f), -(((float)nDepth) * fTileZ / 2.0f));
	psCelticData->psRenderData->psBezData = psBezData;
	
	psCelticData->aeCorner = NULL;
	psCelticData->aeCentre = NULL;

	/* Set up the virtual functions */
	psCelticData->DeleteCelticPersist = & DeleteCelticPersist3D;
	
	psCelticData->GenerateKnot = & GenerateKnot3D;
	psCelticData->RenderKnots = & RenderKnots3D;

	psCelticData->GetVolume = & GetVolume3D;

	psCelticData->SaveSettingsCeltic = & SaveSettingsCeltic3D;
	psCelticData->LoadSettingsStartCeltic =  LoadSettingsStartCeltic3D;
	psCelticData->LoadSettingsEndCeltic =  LoadSettingsEndCeltic3D;
	psCelticData->ExportModel = & ExportModel3D;

	return psCelticData;
}

void DeleteCelticPersist3D (CelticPersist * psCelticData) {
	if (psCelticData) {
		if (psCelticData->aeCorner) {
			free (psCelticData->aeCorner);
			psCelticData->aeCorner = NULL;
		}
		if (psCelticData->aeCentre) {
			free (psCelticData->aeCentre);
			psCelticData->aeCentre = NULL;
		}
		if (psCelticData->psRenderData) {
			DeleteRenderPersist (psCelticData->psRenderData);
			psCelticData->psRenderData = NULL;
		}

		free (psCelticData);
		psCelticData = NULL;
	}
}

/* Main function */
void GenerateKnot3D (CelticPersist * psCelticData) {
	int nBezier;
	int nBeziers;

	/* Re-seed the random number generator */
	//psCelticData->uSeed = 0;
	srand (psCelticData->uSeed);
	// Random number cycle correction
	rand ();

	if (psCelticData->aeCorner) {
		free (psCelticData->aeCorner);
	}
	if (psCelticData->aeCentre) {
		free (psCelticData->aeCentre);
	}
	psCelticData->aeCorner = (TILE *)calloc (((psCelticData->vnSize.nY + 1) * (psCelticData->vnSize.nX + 1) * (psCelticData->vnSize.nZ + 1)), sizeof (TILE));
	psCelticData->aeCentre = (TILE *)calloc (((psCelticData->vnSize.nY) * (psCelticData->vnSize.nX) * (psCelticData->vnSize.nZ)), sizeof (TILE));

	/* Create the random knot tiles */
	PopulateTiles (psCelticData->eOrientation, psCelticData);

	if (psCelticData->boSymmetrify) {
		SymmetrifyTiles (psCelticData);
	}

	psCelticData->nLoops = ColourTiles (psCelticData);

	/* Remove any beziers if there are any */
	if (psCelticData->psRenderData->nBezierNum > 0) {
		DeleteBeziers (psCelticData->psRenderData->psBezierStart, psCelticData->psRenderData->nBezierNum, psCelticData->psRenderData->psBezData);
		psCelticData->psRenderData->psBezierStart = NULL;
		psCelticData->psRenderData->nBezierNum = 0;
	}

	/* Generate bezier tubes */
	nBeziers = (psCelticData->vnSize.nX * psCelticData->vnSize.nY * psCelticData->vnSize.nZ * BEZIERS_PER_TILE);
	psCelticData->psRenderData->nBezierNum = nBeziers;
	if (nBeziers > 0) {
		psCelticData->psRenderData->psBezierStart = NewBezier (psCelticData->psRenderData->psBezData);

		for (nBezier = 0; nBezier < (nBeziers - 1); nBezier++) {
			NewBezier (psCelticData->psRenderData->psBezData);
		}
	}
}

/* Find the next piece of knot after the given piece, based on the direction of the string */
static void FollowKnotInside (VecInt3 * pvnPos, VecInt3 * pvnCorner, CelticPersist * psCelticData) {
	TILE eCentre;

	if (pvnPos && pvnCorner) {
		if ((pvnPos->nX >= 0) && (pvnPos->nY >= 0) && (pvnPos->nZ >= 0) && (pvnPos->nX < psCelticData->vnSize.nX) && (pvnPos->nY < psCelticData->vnSize.nY) && (pvnPos->nZ < psCelticData->vnSize.nZ)) {
			eCentre = GetCentre (pvnPos, psCelticData);

			switch (eCentre) {
				case TILE_HORIZONTAL:
					pvnCorner->nX = 1 - pvnCorner->nX;
					break;
				case TILE_VERTICAL:
					pvnCorner->nY = 1 - pvnCorner->nY;
					break;
				case TILE_LONGITUDINAL:
					pvnCorner->nZ = 1 - pvnCorner->nZ;
					break;
				case TILE_CROSS:
					pvnCorner->nX = 1 - pvnCorner->nX;
					pvnCorner->nY = 1 - pvnCorner->nY;
					pvnCorner->nZ = 1 - pvnCorner->nZ;
					break;
				default:
					// Do nothing
					g_assert (FALSE);
					break;
			}
		}
	}
}

/* Find the next piece of knot after the given piece, based on the direction of the string */
static void FollowKnotToNext (VecInt3 * pvnPos, VecInt3 * pvnCorner, CelticPersist * psCelticData) {
	TILE eCorner;

	if (pvnPos && pvnCorner) {
		if ((pvnPos->nX >= 0) && (pvnPos->nY >= 0) && (pvnPos->nZ >= 0) && (pvnPos->nX < psCelticData->vnSize.nX) && (pvnPos->nY < psCelticData->vnSize.nY) && (pvnPos->nZ < psCelticData->vnSize.nZ)) {
			eCorner = GetCorner (pvnPos, pvnCorner, psCelticData);
			switch (eCorner) {
				case TILE_HORIZONTAL:
					pvnPos->nX += ((pvnCorner->nX > 0) ? 1 : -1);
					pvnCorner->nX = (1 - pvnCorner->nX);
					break;
				case TILE_VERTICAL:
					pvnPos->nY += ((pvnCorner->nY > 0) ? 1 : -1);
					pvnCorner->nY = (1 - pvnCorner->nY);
					break;
				case TILE_LONGITUDINAL:
					pvnPos->nZ += ((pvnCorner->nZ > 0) ? 1 : -1);
					pvnCorner->nZ = (1 - pvnCorner->nZ);
					break;
				case TILE_CROSS:
					pvnPos->nX += ((pvnCorner->nX > 0) ? 1 : -1);
					pvnCorner->nX = (1 - pvnCorner->nX);
					pvnPos->nY += ((pvnCorner->nY > 0) ? 1 : -1);
					pvnCorner->nY = (1 - pvnCorner->nY);
					pvnPos->nZ += ((pvnCorner->nZ > 0) ? 1 : -1);
					pvnCorner->nZ = (1 - pvnCorner->nZ);
					break;
				default:
					// Do nothing
					break;
			}
		}
	}
}

static void ColourLoop (VecInt3 * pvnPos, VecInt3 * pvnCorner, Cube3Complete * asCompleted, ColFloats const * psColour, CelticPersist * psCelticData) {
	int nIndex;
	int nCornerIndex;
	VecInt3 vnUnitSize;

	SetVecInt3 (vnUnitSize, 2, 2, 2);

	nIndex = ConvertToIndex (pvnPos, & psCelticData->vnSize);
	nCornerIndex = ConvertToIndex (pvnCorner, & vnUnitSize);

	do {
		asCompleted[nIndex].aCorner[nCornerIndex] = TRUE;
		FollowKnotInside (pvnPos, pvnCorner, psCelticData);

		nIndex = ConvertToIndex (pvnPos, & psCelticData->vnSize);
		nCornerIndex = ConvertToIndex (pvnCorner, & vnUnitSize);
		psCelticData->psRenderData->asColour3D[nIndex].aCorner[nCornerIndex] = * psColour;
		asCompleted[nIndex].aCorner[nCornerIndex] = TRUE;

		FollowKnotToNext (pvnPos, pvnCorner, psCelticData);

		nIndex = ConvertToIndex (pvnPos, & psCelticData->vnSize);
		nCornerIndex = ConvertToIndex (pvnCorner, & vnUnitSize);
		psCelticData->psRenderData->asColour3D[nIndex].aCorner[nCornerIndex] = * psColour;
	} while (asCompleted[nIndex].aCorner[nCornerIndex] != TRUE);
}

/* Define a colour for each of the pieces of the string */
static int ColourTiles (CelticPersist * psCelticData) {
	VecInt3 vnPos;
	Cube3Complete * asCompleted;
	int nCount;
	int nTotal;
	int nCorner;
	int nIndex;
	VecInt3 vnLoopPos;
	VecInt3 vnLoopCorner;
	int nLoops;
	TILE eCorner;
	ColFloats sColour;

	srand (psCelticData->psRenderData->uColourSeed);

	if (psCelticData->psRenderData->asColour3D) {
		free (psCelticData->psRenderData->asColour3D);
	}
	nTotal = (psCelticData->vnSize.nY * psCelticData->vnSize.nX * psCelticData->vnSize.nZ);
	psCelticData->psRenderData->asColour3D = (Cube3Colour *)malloc ((nTotal * sizeof (Cube3Colour)));

	asCompleted = (Cube3Complete *)malloc (nTotal * sizeof (Cube3Complete));

	for (nCount = 0; nCount < nTotal; nCount++) {
		for (nCorner = 0; nCorner < 8; nCorner++) {
			asCompleted[nCount].aCorner[nCorner] = FALSE;
			psCelticData->psRenderData->asColour3D[nCount].aCorner[nCorner].fRed = 0.0f;
			psCelticData->psRenderData->asColour3D[nCount].aCorner[nCorner].fGreen = 0.0f;
			psCelticData->psRenderData->asColour3D[nCount].aCorner[nCorner].fBlue = 0.0f;
		}
	}

	nLoops = 0;
	for (vnPos.nX = 0; vnPos.nX < psCelticData->vnSize.nX; vnPos.nX++) {
		for (vnPos.nY = 0; vnPos.nY < psCelticData->vnSize.nY; vnPos.nY++) {
			for (vnPos.nZ = 0; vnPos.nZ < psCelticData->vnSize.nZ; vnPos.nZ++) {
				for (nCorner = 0; nCorner < 8; nCorner++) {
					nIndex = ConvertToIndex (& vnPos, & psCelticData->vnSize);
					if (asCompleted[nIndex].aCorner[nCorner] == FALSE) {
						vnLoopPos = vnPos;
						SetVecInt3 (vnLoopCorner, (nCorner % 2), ((nCorner / 2) % 2), (nCorner / 4));
						eCorner = GetCorner (& vnLoopPos, & vnLoopCorner, psCelticData);
						SelectColour (& sColour, psCelticData->psRenderData);
						if (eCorner != TILE_INVALID) {
							ColourLoop (& vnLoopPos, & vnLoopCorner, asCompleted, & sColour, psCelticData);
							nLoops++;
						}
					}
				}
			}
		}
	}

	free (asCompleted);
	return nLoops;
}

static void SelectColour (ColFloats * psColour, RenderPersist * psRenderData) {
	if (psColour) {
		if (psRenderData->uColourSeed != 0) {
			psColour->fRed = ((float)(rand () % 0xff)) / 255.0f;
			psColour->fGreen = ((float)(rand () % 0xff)) / 255.0f;
			psColour->fBlue = ((float)(rand () % 0xff)) / 255.0f;
		}
		else {
			psColour->fRed = COLOUR_PLAIN_RED;
			psColour->fGreen = COLOUR_PLAIN_GREEN;
			psColour->fBlue = COLOUR_PLAIN_BLUE;
		}
	}
}

/* Get the tile direction for a given corner */
static TILE GetCorner (VecInt3 const * pvnPos, VecInt3 const * pvnCorner, CelticPersist * psCelticData) {
	VecInt3 vnIndex;
	VecInt3 vnSize;
	int nIndex;
	TILE eReturn = TILE_INVALID;

	if (psCelticData && psCelticData->aeCorner) {
		vnIndex = AddVecInts (pvnPos, pvnCorner);
		vnSize.nX = psCelticData->vnSize.nX + 1;
		vnSize.nY = psCelticData->vnSize.nY + 1;
		vnSize.nZ = psCelticData->vnSize.nZ + 1;

		nIndex = ConvertToIndex (& vnIndex, & vnSize);

		if (nIndex >= 0) {
			eReturn = psCelticData->aeCorner[nIndex];
		}
	}

	return eReturn;
}

/* Set the tile direction for a given corner */
static void SetCorner (VecInt3 const * pvnPos, VecInt3 const * pvnCorner, TILE eValue, CelticPersist * psCelticData) {
	VecInt3 vnIndex;
	VecInt3 vnSize;
	int nIndex;

	if (psCelticData && psCelticData->aeCorner) {
		vnIndex = AddVecInts (pvnPos, pvnCorner);
		vnSize.nX = psCelticData->vnSize.nX + 1;
		vnSize.nY = psCelticData->vnSize.nY + 1;
		vnSize.nZ = psCelticData->vnSize.nZ + 1;

		nIndex = ConvertToIndex (& vnIndex, & vnSize);

		if (nIndex >= 0) {
			psCelticData->aeCorner[nIndex] = eValue;
		}
	}
}

/* Get the tile direction for the centre of a given tile */
static TILE GetCentre (VecInt3 const * pvnPos, CelticPersist * psCelticData) {
	int nIndex;
	TILE eReturn = TILE_INVALID;

	if (psCelticData && psCelticData->aeCentre) {
		nIndex = ConvertToIndex (pvnPos, & psCelticData->vnSize);

		if (nIndex >= 0) {
			eReturn = psCelticData->aeCentre[nIndex];
		}
	}

	return eReturn;
}

/* Set the tile direction for the centre of a given tile */
static void SetCentre (VecInt3 const * pvnPos, TILE eValue, CelticPersist * psCelticData) {
	int nIndex;

	if (psCelticData && psCelticData->aeCentre) {
		nIndex = ConvertToIndex (pvnPos, & psCelticData->vnSize);

		if (nIndex >= 0) {
			psCelticData->aeCentre[nIndex] = eValue;
		}
	}
}

/* Poplulate the tiles with a random set of directions */
static void PopulateTiles (TILE eOrientation, CelticPersist * psCelticData) {
	int nXPos;
	int nYPos;
	int nZPos;
	int nCorner;
	TILE eTileChoice;
	float fCrossRand;
	VecInt3 vnPos;
	VecInt3 vnCorner;
	TILE aeChoice[3];

	/* Populate the edge and corner arrays */
	for (nXPos = 1; nXPos < (psCelticData->vnSize.nX - 1); nXPos++) {
		for (nYPos = 1; nYPos < (psCelticData->vnSize.nY - 1); nYPos++) {
			for (nZPos = 1; nZPos < (psCelticData->vnSize.nZ - 1); nZPos++) {
				SetVecInt3 (vnPos, nXPos, nYPos, nZPos);
				for (nCorner = 0; nCorner < 8; nCorner++) {
					SetVecInt3 (vnCorner, (nCorner % 2), ((nCorner / 2) % 2), (nCorner / 4));
					eTileChoice = (TILE)(rand () % (int)TILE_NUM);
					SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
				}

				fCrossRand = ((float)rand () / (float)RAND_MAX);
				eTileChoice = (TILE)((rand () % ((int)TILE_NUM - 1)) + 1);
				if (fCrossRand >= psCelticData->fWeirdness) {
					eTileChoice = TILE_CROSS;
				}
				SetCentre (& vnPos, eTileChoice, psCelticData);
			}
		}
	}

	/* Deal with the x-z sides */
	aeChoice[0] = TILE_LONGITUDINAL;
	aeChoice[1] = TILE_HORIZONTAL;
	for (nXPos = 0; nXPos < (psCelticData->vnSize.nX - 0); nXPos++) {
		for (nZPos = 0; nZPos < (psCelticData->vnSize.nZ - 0); nZPos++) {
			SetVecInt3 (vnPos, nXPos, 0, nZPos);

			SetVecInt3 (vnCorner, 0, 0, 0);
			SetCorner (& vnPos, & vnCorner, TILE_INVALID, psCelticData);
			SetVecInt3 (vnCorner, 1, 0, 0);
			SetCorner (& vnPos, & vnCorner, TILE_INVALID, psCelticData);

			SetVecInt3 (vnCorner, 0, 0, 1);
			SetCorner (& vnPos, & vnCorner, TILE_INVALID, psCelticData);
			SetVecInt3 (vnCorner, 1, 0, 1);
			SetCorner (& vnPos, & vnCorner, TILE_INVALID, psCelticData);

			eTileChoice = SelectRandom (aeChoice, 2);
			SetCentre (& vnPos, eTileChoice, psCelticData);


			SetVecInt3 (vnPos, nXPos, (psCelticData->vnSize.nY - 1), nZPos);

			SetVecInt3 (vnCorner, 0, 1, 0);
			SetCorner (& vnPos, & vnCorner, TILE_INVALID, psCelticData);
			SetVecInt3 (vnCorner, 1, 1, 0);
			SetCorner (& vnPos, & vnCorner, TILE_INVALID, psCelticData);

			SetVecInt3 (vnCorner, 0, 1, 1);
			SetCorner (& vnPos, & vnCorner, TILE_INVALID, psCelticData);
			SetVecInt3 (vnCorner, 1, 1, 1);
			SetCorner (& vnPos, & vnCorner, TILE_INVALID, psCelticData);

			eTileChoice = SelectRandom (aeChoice, 2);
			SetCentre (& vnPos, eTileChoice, psCelticData);
		}
	}

	/* Deal with the x-z sides */
	aeChoice[0] = TILE_CROSS;
	aeChoice[1] = TILE_CROSS;
	aeChoice[2] = TILE_CROSS;
	for (nXPos = 1; nXPos < (psCelticData->vnSize.nX - 1); nXPos++) {
		for (nZPos = 1; nZPos < (psCelticData->vnSize.nZ - 1); nZPos++) {
			SetVecInt3 (vnPos, nXPos, 0, nZPos);

			SetVecInt3 (vnCorner, 0, 1, 0);
			eTileChoice = SelectRandom (aeChoice, 3);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 1, 1, 0);
			eTileChoice = SelectRandom (aeChoice, 3);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

			SetVecInt3 (vnCorner, 0, 1, 1);
			eTileChoice = SelectRandom (aeChoice, 3);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 1, 1, 1);
			eTileChoice = SelectRandom (aeChoice, 3);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);


			SetVecInt3 (vnPos, nXPos, (psCelticData->vnSize.nY - 1), nZPos);

			SetVecInt3 (vnCorner, 0, 0, 0);
			eTileChoice = SelectRandom (aeChoice, 3);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 1, 0, 0);
			eTileChoice = SelectRandom (aeChoice, 3);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

			SetVecInt3 (vnCorner, 0, 0, 1);
			eTileChoice = SelectRandom (aeChoice, 3);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 1, 0, 1);
			eTileChoice = SelectRandom (aeChoice, 3);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		}
	}

	/* Deal with the y-z sides */
	aeChoice[0] = TILE_VERTICAL;
	aeChoice[1] = TILE_LONGITUDINAL;
	for (nYPos = 1; (nYPos < psCelticData->vnSize.nY - 1); nYPos++) {
		for (nZPos = 1; (nZPos < psCelticData->vnSize.nZ - 1); nZPos++) {
			SetVecInt3 (vnPos, 0, nYPos, nZPos);

			SetVecInt3 (vnCorner, 0, 0, 0);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 0, 1, 0);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

			SetVecInt3 (vnCorner, 0, 0, 1);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 0, 1, 1);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

			SetVecInt3 (vnPos, (psCelticData->vnSize.nX - 1), nYPos, nZPos);

			SetVecInt3 (vnCorner, 1, 0, 0);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 1, 1, 0);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

			SetVecInt3 (vnCorner, 1, 0, 1);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 1, 1, 1);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		}
	}

	/* Deal with the x-y sides */
	aeChoice[0] = TILE_VERTICAL;
	aeChoice[1] = TILE_HORIZONTAL;
	for (nXPos = 1; (nXPos < psCelticData->vnSize.nX - 1); nXPos++) {
		for (nYPos = 1; (nYPos < psCelticData->vnSize.nY - 1); nYPos++) {
			SetVecInt3 (vnPos, nXPos, nYPos, 0);

			SetVecInt3 (vnCorner, 0, 0, 0);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 0, 1, 0);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

			SetVecInt3 (vnCorner, 1, 0, 0);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 1, 1, 0);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

			SetVecInt3 (vnPos, nXPos, nYPos, (psCelticData->vnSize.nZ - 1));

			SetVecInt3 (vnCorner, 0, 0, 1);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 0, 1, 1);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

			SetVecInt3 (vnCorner, 1, 0, 1);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
			SetVecInt3 (vnCorner, 1, 1, 1);
			eTileChoice = SelectRandom (aeChoice, 2);
			SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		}
	}

	/* Deal with the x edges */
	eTileChoice = TILE_VERTICAL;
	for (nXPos = 1; (nXPos < psCelticData->vnSize.nX - 1); nXPos++) {
		SetVecInt3 (vnPos, nXPos, 0, 0);
		SetVecInt3 (vnCorner, 0, 1, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 1, 1, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

		SetVecInt3 (vnPos, nXPos, (psCelticData->vnSize.nY - 1), 0);
		SetVecInt3 (vnCorner, 0, 0, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 1, 0, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

		SetVecInt3 (vnPos, nXPos, 0, (psCelticData->vnSize.nZ - 1));
		SetVecInt3 (vnCorner, 0, 1, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 1, 1, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

		SetVecInt3 (vnPos, nXPos, (psCelticData->vnSize.nY - 1), (psCelticData->vnSize.nZ - 1));
		SetVecInt3 (vnCorner, 0, 0, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 1, 0, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
	}

	/* Deal with the y edges */
	eTileChoice = TILE_VERTICAL;
	for (nYPos = 1; (nYPos < psCelticData->vnSize.nY - 1); nYPos++) {
		SetVecInt3 (vnPos, 0, nYPos, 0);
		SetVecInt3 (vnCorner, 0, 0, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 0, 1, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

		SetVecInt3 (vnPos, (psCelticData->vnSize.nX - 1), nYPos, 0);
		SetVecInt3 (vnCorner, 1, 0, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 1, 1, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

		SetVecInt3 (vnPos, 0, nYPos, (psCelticData->vnSize.nZ - 1));
		SetVecInt3 (vnCorner, 0, 0, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 0, 1, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

		SetVecInt3 (vnPos, (psCelticData->vnSize.nX - 1), nYPos, (psCelticData->vnSize.nZ - 1));
		SetVecInt3 (vnCorner, 1, 0, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 1, 1, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
	}

	/* Deal with the z edges */
	eTileChoice = TILE_VERTICAL;
	for (nZPos = 1; (nZPos < psCelticData->vnSize.nZ - 1); nZPos++) {
		SetVecInt3 (vnPos, 0, 0, nZPos);
		SetVecInt3 (vnCorner, 0, 1, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 0, 1, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

		SetVecInt3 (vnPos, (psCelticData->vnSize.nX - 1), 0, nZPos);
		SetVecInt3 (vnCorner, 1, 1, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 1, 1, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

		SetVecInt3 (vnPos, 0, (psCelticData->vnSize.nY - 1), nZPos);
		SetVecInt3 (vnCorner, 0, 0, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 0, 0, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);

		SetVecInt3 (vnPos, (psCelticData->vnSize.nX - 1), (psCelticData->vnSize.nY - 1), nZPos);
		SetVecInt3 (vnCorner, 1, 0, 0);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
		SetVecInt3 (vnCorner, 1, 0, 1);
		SetCorner (& vnPos, & vnCorner, eTileChoice, psCelticData);
	}
}

/* Rotate a tile location around a given axis based on the size of the box */
static VecInt3 RotateTile (VecInt3 const * pvnPos, VecInt3 const * pvnSize, VecInt3 const * pvnAxis) {
	VecInt3 vnResult;

	vnResult = * pvnPos;

	/* Always rotate by half a circle */
	if (pvnAxis->nX != 0) {
		// Reflect y and z
		vnResult.nY = (pvnSize->nY - 1) - vnResult.nY;
		vnResult.nZ = (pvnSize->nZ - 1) - vnResult.nZ;
	}
	if (pvnAxis->nY != 0) {
		// Reflect x and z
		vnResult.nX = (pvnSize->nX - 1) - vnResult.nX;
		vnResult.nZ = (pvnSize->nZ - 1) - vnResult.nZ;
	}
	if (pvnAxis->nZ != 0) {
		// Reflect x and y
		vnResult.nX = (pvnSize->nX - 1) - vnResult.nX;
		vnResult.nY = (pvnSize->nY - 1) - vnResult.nY;
	}

	return vnResult;
}

/* Fills out the array with the four positions: the original tile and the tile rotated around each axis */
static void FindRotatedTiles (VecInt3 const * pvnPos, VecInt3 const * pvnSize, VecInt3 * anRotatedPos, int nArraySize) {
	int nIndex = 0;
	int nXPos;
	int nYPos;
	int nZPos;

	/* No rotation */
	if (nArraySize > nIndex) {
		nXPos = pvnPos->nX;
		nYPos = pvnPos->nY;
		nZPos = pvnPos->nZ;
		SetVecInt3 (anRotatedPos[nIndex], nXPos, nYPos, nZPos);
	}

	/* Rotation about x-axis */
	/* The y-z faces */
	nIndex++;
	if (nArraySize > nIndex) {
		nXPos = pvnPos->nX;
		nYPos = (pvnSize->nY - 1) - pvnPos->nY;
		nZPos = (pvnSize->nZ - 1) - pvnPos->nZ;
		SetVecInt3 (anRotatedPos[nIndex], nXPos, nYPos, nZPos);
	}

	/* Rotation about y-axis */
	/* The x-z faces */
	nIndex++;
	if (nArraySize > nIndex) {
		nXPos = (pvnSize->nX - 1) - pvnPos->nX;
		nYPos = pvnPos->nY;
		nZPos = (pvnSize->nZ - 1) - pvnPos->nZ;
		SetVecInt3 (anRotatedPos[nIndex], nXPos, nYPos, nZPos);
	}

	/* Rotation about z-axis */
	/* The x-y faces */
	nIndex++;
	if (nArraySize > nIndex) {
		nXPos = (pvnSize->nX - 1) - pvnPos->nX;
		nYPos = (pvnSize->nY - 1) - pvnPos->nY;
		nZPos = pvnPos->nZ;
		SetVecInt3 (anRotatedPos[nIndex], nXPos, nYPos, nZPos);
	}
}

/* Alter the tile directions to create a rotationally symmetric knot */
static void SymmetrifyTiles (CelticPersist * psCelticData) {
	TILE eTileCorner;
	TILE eTileCentre;
	VecInt3 vnRotate;
	VecInt3 vnPos;
	VecInt3 vnCorner;
	VecInt3 vnCornerRotated;
	VecInt3 vnUnitSize;
	int nCorner;
	VecInt3 anRotatedPos[4];
	int nRotated;

	SetVecInt3 (vnUnitSize, 2, 2, 2);
	for (vnPos.nX = 0; vnPos.nX < psCelticData->vnSize.nX; vnPos.nX++) {
		for (vnPos.nY = 0; vnPos.nY < psCelticData->vnSize.nY; vnPos.nY++) {
			for (vnPos.nZ = 0; vnPos.nZ < psCelticData->vnSize.nZ; vnPos.nZ++) {
				FindRotatedTiles (& vnPos, & psCelticData->vnSize, anRotatedPos, sizeof (anRotatedPos));
				for (nRotated = 0; nRotated < 4; nRotated++) {
					eTileCentre = GetCentre (& vnPos, psCelticData);
					SetCentre (& anRotatedPos[nRotated], eTileCentre, psCelticData);

					SetVecInt3 (vnRotate, (nRotated == 1), (nRotated == 2), (nRotated == 3));
					for (nCorner = 0; nCorner < 8; nCorner++) {
						SetVecInt3 (vnCorner, (nCorner % 2), ((nCorner / 2) % 2), (nCorner / 4));
						eTileCorner = GetCorner (& vnPos, & vnCorner, psCelticData);

						vnCornerRotated = RotateTile (& vnCorner, & vnUnitSize, & vnRotate);
						SetCorner (& anRotatedPos[nRotated], & vnCornerRotated, eTileCorner, psCelticData);
					}
				}
			}
		}
	}
}

/* Render the knots to the canvas */
void RenderKnots3D (CelticPersist * psCelticData) {
	int nXPos;
	int nYPos;
	int nZPos;
	int nCorner;
	TILE eTileCentre;
	TILE aeTileEdge[2][2][2];
	VecInt3 vnPos;
	VecInt3 vnCorner;
	Vector3 vPos;
	Cube3Colour sColours;
	int nIndex;

	/* Sanity check */
	g_assert ((psCelticData->psRenderData->nBezierNum == (psCelticData->vnSize.nX * psCelticData->vnSize.nY * psCelticData->vnSize.nZ * BEZIERS_PER_TILE)));
	g_assert (psCelticData->psRenderData->psBezierStart);
	psCelticData->psRenderData->psBezierCurrent = psCelticData->psRenderData->psBezierStart;
	
	// Reset the length for volume calculation
	psCelticData->psRenderData->fLength = 0.0f;

	// Clear the currently stored beziers if there are any
	StoreBeziers (FALSE, psCelticData->psRenderData->psBezData);

	// Set the next set of beziers to be stored
	StoreBeziers (TRUE, psCelticData->psRenderData->psBezData);

	/* Draw the knots */
	for (nZPos = 0; nZPos < psCelticData->vnSize.nZ; nZPos++) {
		for (nXPos = 0; nXPos < psCelticData->vnSize.nX; nXPos++) {
			for (nYPos = 0; nYPos < psCelticData->vnSize.nY; nYPos++) {
				SetVecInt3 (vnPos, nXPos, nYPos, nZPos);
				for (nCorner = 0; nCorner < 8; nCorner++) {
					SetVecInt3 (vnCorner, (nCorner % 2), ((nCorner / 2) % 2), (nCorner / 4));
					aeTileEdge[(nCorner % 2)][((nCorner / 2) % 2)][(nCorner / 4)] = GetCorner (& vnPos, & vnCorner, psCelticData);
				}
				nIndex = ConvertToIndex (& vnPos, & psCelticData->vnSize);
				sColours = psCelticData->psRenderData->asColour3D[nIndex];

				eTileCentre = GetCentre (& vnPos, psCelticData);
				SetVector3 (vPos, (nXPos * psCelticData->vTileSize.fX), (nYPos * psCelticData->vTileSize.fY), (nZPos * psCelticData->vTileSize.fZ));
				TileKnot (& vPos, & psCelticData->vTileSize, aeTileEdge, eTileCentre, & sColours, psCelticData->psRenderData);
			}
		}
	}
}

bool ExportModel3D (char const * szFilename, bool boBinary, CelticPersist * psCelticData) {
	bool boSuccess;

	boSuccess = OutputStoredBeziers (szFilename, psCelticData->psRenderData->psBezData, boBinary);

	return boSuccess;
}

/* Render a bezier curve */
/* We now use ImageBezierSplit instead */
/*
static void ImageBezier (Vector3 const * pvStart, Vector3 const * pvStartDir, Vector3 const * pvEnd, Vector3 const * pvEndDir, RenderPersist * psRenderData) {
	Vector3 vStart;
	Vector3 vStartDir;
	Vector3 vEnd;
	Vector3 vEndDir;

	vStart = AddVectors (pvStart, & psRenderData->vOffset);
	vStartDir = AddVectors (pvStartDir, & psRenderData->vOffset);
	vEnd = AddVectors (pvEnd, & psRenderData->vOffset);
	vEndDir = AddVectors (pvEndDir, & psRenderData->vOffset);

	SetBezierControlPoints (psRenderData->fThickness, vStart, vStartDir, vEnd, vEndDir, NULL, NULL, psRenderData->psBezierCurrent, psRenderData->psBezData);
	psRenderData->psBezierCurrent = GetBezierNext (psRenderData->psBezierCurrent);
}
*/

/* Render a bezier curve pushed forwards or backwards in the middle */
static void ImageBezierSplit (Vector3 const * pvStart, Vector3 const * pvStartDir, Vector3 const * pvEnd, Vector3 const * pvEndDir, float fRatio, Vector3 const * pvWeave, Vector3 const * pvOverride, ColFloats const * psStartColour, ColFloats const * psEndColour, RenderPersist * psRenderData) {
	Vector3 vStart;
	Vector3 vStartDir;
	Vector3 vEnd;
	Vector3 vEndDir;
	Vector3 vMid;
	Vector3 vMidDirBack;
	Vector3 vMidDirForward;
	Vector3 vShift;
	ColFloats sMidColour;
	ColFloats * psMidColour;

	if ((psStartColour != NULL) && (psEndColour != NULL)) {
		/* Red */
		sMidColour.fRed = (psStartColour->fRed + psEndColour->fRed) / 2.0f;
		/* Green */
		sMidColour.fGreen = (psStartColour->fGreen + psEndColour->fGreen) / 2.0f;
		/* Blue */
		sMidColour.fBlue = (psStartColour->fBlue + psEndColour->fBlue) / 2.0f;
		psMidColour = & sMidColour;
	}
	else {
		psMidColour = NULL;
	}
	
	SetVector3 (vShift, 0.0f, 0.0f, 0.0f);
	vStart = AddVectors (pvStart, & psRenderData->vOffset);
	vStartDir = AddVectors (pvStartDir, & psRenderData->vOffset);
	vEnd = AddVectors (pvEnd, & psRenderData->vOffset);
	vEndDir = AddVectors (pvEndDir, & psRenderData->vOffset);

	SplitBezier (vStart, & vStartDir, vEnd, & vEndDir, fRatio, & vMid, & vMidDirBack, & vMidDirForward);

	if (pvOverride != NULL) {
		vShift = AddVectors (pvOverride, & psRenderData->vOffset);
		vShift.fX -= vMid.fX;
		vShift.fY -= vMid.fY;
		vShift.fZ -= vMid.fZ;
	}

	vMid = AddVectors (& vMid, pvWeave);
	vMidDirBack = AddVectors (& vMidDirBack, pvWeave);
	vMidDirForward = AddVectors (& vMidDirForward, pvWeave);

	vMid = AddVectors (& vMid, & vShift);
	vMidDirBack = AddVectors (& vMidDirBack, & vShift);
	vMidDirForward = AddVectors (& vMidDirForward, & vShift);

	g_assert (psRenderData->psBezierCurrent);
	SetBezierControlPoints (psRenderData->fThickness, vStart, vStartDir, vMid, vMidDirBack, (float const *)psStartColour, (float const *)(& sMidColour), psRenderData->psBezierCurrent, psRenderData->psBezData);
	psRenderData->psBezierCurrent = GetBezierNext (psRenderData->psBezierCurrent);

	g_assert (psRenderData->psBezierCurrent);
	SetBezierControlPoints (psRenderData->fThickness, vMid, vMidDirForward, vEnd, vEndDir, (float const *)psMidColour, (float const *)psEndColour, psRenderData->psBezierCurrent, psRenderData->psBezData);
	psRenderData->psBezierCurrent = GetBezierNext (psRenderData->psBezierCurrent);
}

static Vector3 GetWeaveCorner (VecInt3 const * pvnCorner, RenderPersist * psRenderData) {
	Vector3 vResult;
	static unsigned int const auWeave[8] = {7, 2, 4, 1, 1, 4, 2, 7};
	int nIndex;
	unsigned int uCorner;
	
	nIndex = ((pvnCorner->nX) + (2 * (pvnCorner->nY)) + (4 * (pvnCorner->nZ))) % 8;
	uCorner = auWeave[nIndex];

	SetVector3 (vResult, (uCorner % 2), ((uCorner / 2) % 2), (uCorner / 4));
	vResult = ScaleVector (& vResult, psRenderData->fWeaveHeight);

	return vResult;
}

static Vector3 GetWeaveCentre (VecInt3 const * pvnCorner, RenderPersist * psRenderData) {
	Vector3 vResult;
	static unsigned int const auWeave[8] = {7, 2, 4, 1, 1, 4, 2, 7};
	int nIndex;
	unsigned int uCorner;
	
	nIndex = ((pvnCorner->nX) + (2 * (pvnCorner->nY)) + (4 * (pvnCorner->nZ))) % 8;
	uCorner = auWeave[nIndex];

	SetVector3 (vResult, (uCorner % 2), ((uCorner / 2) % 2), (uCorner / 4));
	vResult = ScaleVector (& vResult, -psRenderData->fWeaveHeight);

	return vResult;
}

/* Render a single bezier curve given the parameters of a tile */
static void TileLine (Vector3 const * pvPos, Vector3 const * pvSize, VecInt3 const * pvnCornerStart, VecInt3 const * pvnCornerEnd, TILE eStartDir, TILE eEndDir, ColFloats const * psStartColour, ColFloats const * psEndColour, RenderPersist * psRenderData) {
	Vector3 vPos[2];
	Vector3 vDir[2];
	Vector3 vShift[2];
	Vector3 vWeave[2];
	Vector3 vWeaveMid;
	Vector3 vInvert;
	Vector3 vOverride;
	Vector3 * pvOverride;
	int nCount;
	bool boDraw = TRUE;

	for (nCount = 0; nCount < 2; nCount++) {
		SetVector3 (vPos[nCount], 0.0f, 0.0f, 0.0f);
		SetVector3 (vDir[nCount], 0.0f, 0.0f, 0.0f);
		SetVector3 (vShift[nCount], 0.0f, 0.0f, 0.0f);
		SetVector3 (vWeave[nCount], 0.0f, 0.0f, 0.0f);
	}
	SetVector3 (vInvert, 0.0f, 0.0f, 0.0f);
	SetVector3 (vWeaveMid, 0.0f, 0.0f, 0.0f);

	switch (eStartDir) {
	case TILE_HORIZONTAL:
		vDir[0].fX = (pvSize->fX * psRenderData->fControlScale);
		vDir[0].fY = 0.0f;
		vDir[0].fZ = 0.0f;
		vShift[0].fX = 0.0f;
		vShift[0].fY = psRenderData->vLineInset.fY;
		vShift[0].fZ = psRenderData->vLineInset.fZ;
		break;
	case TILE_VERTICAL:
		vDir[0].fX = 0.0f;
		vDir[0].fY = (pvSize->fY * psRenderData->fControlScale);
		vDir[0].fZ = 0.0f;
		vShift[0].fX = psRenderData->vLineInset.fX;
		vShift[0].fY = 0.0f;
		vShift[0].fZ = psRenderData->vLineInset.fZ;
		break;
	case TILE_LONGITUDINAL:
		vDir[0].fX = 0.0f;
		vDir[0].fY = 0.0f;
		vDir[0].fZ = (pvSize->fZ * psRenderData->fControlScale);
		vShift[0].fX = psRenderData->vLineInset.fX;
		vShift[0].fY = psRenderData->vLineInset.fY;
		vShift[0].fZ = 0.0f;
		break;
	case TILE_CROSS:
		vDir[0].fX = (pvSize->fX * psRenderData->fControlScale);
		vDir[0].fY = (pvSize->fY * psRenderData->fControlScale);
		vDir[0].fZ = (pvSize->fZ * psRenderData->fControlScale);
		vShift[0].fX = 0.0f;
		vShift[0].fY = 0.0f;
		vShift[0].fZ = 0.0f;

		vWeave[0] = GetWeaveCorner (pvnCornerStart, psRenderData);
		break;
	case TILE_INVALID:
		boDraw = FALSE;
		break;
	default:
		// Do nothing
		g_assert (FALSE);
		break;
	}

	switch (eEndDir) {
	case TILE_HORIZONTAL:
		vDir[1].fX = (pvSize->fX * psRenderData->fControlScale);
		vDir[1].fY = 0.0f;
		vDir[1].fZ = 0.0f;
		vShift[1].fX = 0.0f;
		vShift[1].fY = psRenderData->vLineInset.fY;
		vShift[1].fZ = psRenderData->vLineInset.fZ;
		break;
	case TILE_VERTICAL:
		vDir[1].fX = 0.0f;
		vDir[1].fY = (pvSize->fY * psRenderData->fControlScale);
		vDir[1].fZ = 0.0f;
		vShift[1].fX = psRenderData->vLineInset.fX;
		vShift[1].fY = 0.0f;
		vShift[1].fZ = psRenderData->vLineInset.fZ;
		break;
	case TILE_LONGITUDINAL:
		vDir[1].fX = 0.0f;
		vDir[1].fY = 0.0f;
		vDir[1].fZ = (pvSize->fZ * psRenderData->fControlScale);
		vShift[1].fX = psRenderData->vLineInset.fX;
		vShift[1].fY = psRenderData->vLineInset.fY;
		vShift[1].fZ = 0.0f;
		break;
	case TILE_CROSS:
		vDir[1].fX = (pvSize->fX * psRenderData->fControlScale);
		vDir[1].fY = (pvSize->fY * psRenderData->fControlScale);
		vDir[1].fZ = (pvSize->fZ * psRenderData->fControlScale);
		vShift[1].fX = 0.0f;
		vShift[1].fY = 0.0f;
		vShift[1].fZ = 0.0f;

		vWeave[1] = GetWeaveCorner (pvnCornerEnd, psRenderData);
		break;
	case TILE_INVALID:
		boDraw = FALSE;
		break;
	default:
		// Do nothing
		g_assert (FALSE);
		break;
	}

	vInvert.fX = ((pvnCornerStart->nX == 0) ? 1.0f : -1.0f);
	vInvert.fY = ((pvnCornerStart->nY == 0) ? 1.0f : -1.0f);
	vInvert.fZ = ((pvnCornerStart->nZ == 0) ? 1.0f : -1.0f);
	vPos[0].fX = pvPos->fX + ((float)pvnCornerStart->nX * pvSize->fX) + (vInvert.fX * vShift[0].fX);
	vPos[0].fY = pvPos->fY + ((float)pvnCornerStart->nY * pvSize->fY) + (vInvert.fY * vShift[0].fY);
	vPos[0].fZ = pvPos->fZ + ((float)pvnCornerStart->nZ * pvSize->fZ) + (vInvert.fZ * vShift[0].fZ);
	vPos[0] = AddVectors (& vPos[0], & vWeave[0]);
	vDir[0].fX = vPos[0].fX + (vInvert.fX * vDir[0].fX);
	vDir[0].fY = vPos[0].fY + (vInvert.fY * vDir[0].fY);
	vDir[0].fZ = vPos[0].fZ + (vInvert.fZ * vDir[0].fZ);

	vInvert.fX = ((pvnCornerEnd->nX == 0) ? 1.0f : -1.0f);
	vInvert.fY = ((pvnCornerEnd->nY == 0) ? 1.0f : -1.0f);
	vInvert.fZ = ((pvnCornerEnd->nZ == 0) ? 1.0f : -1.0f);
	vPos[1].fX = pvPos->fX + ((float)pvnCornerEnd->nX * pvSize->fX) + (vInvert.fX * vShift[1].fX);
	vPos[1].fY = pvPos->fY + ((float)pvnCornerEnd->nY * pvSize->fY) + (vInvert.fY * vShift[1].fY);
	vPos[1].fZ = pvPos->fZ + ((float)pvnCornerEnd->nZ * pvSize->fZ) + (vInvert.fZ * vShift[1].fZ);
	vPos[1] = AddVectors (& vPos[1], & vWeave[1]);
	vDir[1].fX = vPos[1].fX + (vInvert.fX * vDir[1].fX);
	vDir[1].fY = vPos[1].fY + (vInvert.fY * vDir[1].fY);
	vDir[1].fZ = vPos[1].fZ + (vInvert.fZ * vDir[1].fZ);

	vOverride.fX = ((pvPos->fX + ((float)pvnCornerStart->nX * pvSize->fX)) + (pvPos->fX + ((float)pvnCornerEnd->nX * pvSize->fX))) / 2.0f;
	vOverride.fY = ((pvPos->fY + ((float)pvnCornerStart->nY * pvSize->fY)) + (pvPos->fY + ((float)pvnCornerEnd->nY * pvSize->fY))) / 2.0f;
	vOverride.fZ = ((pvPos->fZ + ((float)pvnCornerStart->nZ * pvSize->fZ)) + (pvPos->fZ + ((float)pvnCornerEnd->nZ * pvSize->fZ))) / 2.0f;

	pvOverride = NULL;
	if ((pvnCornerStart->nX != pvnCornerEnd->nX) &&
		(pvnCornerStart->nY != pvnCornerEnd->nY) &&
		(pvnCornerStart->nZ != pvnCornerEnd->nZ)) {
		vWeaveMid = GetWeaveCentre (pvnCornerStart, psRenderData);
		pvOverride = & vOverride;
	}

	if (boDraw) {
		ImageBezierSplit (& vPos[0], & vDir[0], & vPos[1], & vDir[1], 0.5f, & vWeaveMid, pvOverride, psStartColour, psEndColour, psRenderData);

	psRenderData->fLength += BezierCalculateLength (vPos[0], vDir[0], vPos[1], vDir[1], psRenderData->psBezData);
	}
	else {
		SetVector3 (vPos[0], 0.0f, 0.0f, 0.0f);
		SetVector3 (vDir[0], 0.0f, 0.0f, 0.0f);
		SetVector3 (vPos[1], 0.0f, 0.0f, 0.0f);
		SetVector3 (vDir[1], 0.0f, 0.0f, 0.0f);

		ImageBezierSplit (& vPos[0], & vDir[0], & vPos[1], & vDir[1], 0.5f, & vWeaveMid, NULL, NULL, NULL, psRenderData);
	}
}

/* Render a tile */
static void TileKnot (Vector3 const * pvPos, Vector3 const * pvSize, TILE aeEdge[2][2][2], TILE eCentre, Cube3Colour const * psColours, RenderPersist * psRenderData) {
	TILE eStartDir;
	TILE eEndDir;
	VecInt3 vnCornerStart;
	VecInt3 vnCornerEnd;
	ColFloats sStartColour;
	ColFloats sEndColour;
	int nCornerIndex;
	VecInt3 vnCornerSize;

	SetVecInt3 (vnCornerSize, 2, 2, 2);

	SetVecInt3 (vnCornerEnd, 1, 1, 0);
	eEndDir = aeEdge[1][1][0];

	SetVecInt3 (vnCornerStart, 0, 0, 0);
	eStartDir = aeEdge[0][0][0];

	switch (eCentre) {
	case TILE_HORIZONTAL:
		SetVecInt3 (vnCornerStart, 0, 0, 0);
		SetVecInt3 (vnCornerEnd, 1, 0, 0);
		eStartDir = aeEdge[0][0][0];
		eEndDir = aeEdge[1][0][0];
		break;
	case TILE_VERTICAL:
		SetVecInt3 (vnCornerStart, 0, 0, 0);
		SetVecInt3 (vnCornerEnd, 0, 1, 0);
		eStartDir = aeEdge[0][0][0];
		eEndDir = aeEdge[0][1][0];
		break;
	case TILE_LONGITUDINAL:
		SetVecInt3 (vnCornerStart, 0, 0, 0);
		SetVecInt3 (vnCornerEnd, 0, 0, 1);
		eStartDir = aeEdge[0][0][0];
		eEndDir = aeEdge[0][0][1];
		break;
	case TILE_CROSS:
		SetVecInt3 (vnCornerStart, 0, 0, 0);
		SetVecInt3 (vnCornerEnd, 1, 1, 1);
		eStartDir = aeEdge[0][0][0];
		eEndDir = aeEdge[1][1][1];
		break;
	default:
		// Do nothing
		g_assert (FALSE);
		break;
	}

	nCornerIndex = ConvertToIndex (& vnCornerStart, & vnCornerSize);
	sStartColour = psColours->aCorner[nCornerIndex];
	nCornerIndex = ConvertToIndex (& vnCornerEnd, & vnCornerSize);
	sEndColour = psColours->aCorner[nCornerIndex];

	TileLine (pvPos, pvSize, & vnCornerStart, & vnCornerEnd, eStartDir, eEndDir, & sStartColour, & sEndColour, psRenderData);

	switch (eCentre) {
	case TILE_HORIZONTAL:
		SetVecInt3 (vnCornerStart, 0, 1, 0);
		SetVecInt3 (vnCornerEnd, 1, 1, 0);
		//eStart = CORNER_BOTTOMLEFT;
		//eEnd = CORNER_BOTTOMRIGHT;
		eStartDir = aeEdge[0][1][0];
		eEndDir = aeEdge[1][1][0];
		break;
	case TILE_VERTICAL:
		SetVecInt3 (vnCornerStart, 1, 0, 0);
		SetVecInt3 (vnCornerEnd, 1, 1, 0);
		//eStart = CORNER_TOPRIGHT;
		//eEnd = CORNER_BOTTOMRIGHT;
		eStartDir = aeEdge[1][0][0];
		eEndDir = aeEdge[1][1][0];
		break;
	case TILE_LONGITUDINAL:
		SetVecInt3 (vnCornerStart, 1, 0, 0);
		SetVecInt3 (vnCornerEnd, 1, 0, 1);
		//eStart = CORNER_TOPRIGHT;
		//eEnd = CORNER_BOTTOMRIGHT;
		eStartDir = aeEdge[1][0][0];
		eEndDir = aeEdge[1][0][1];
		break;
	case TILE_CROSS:
		SetVecInt3 (vnCornerStart, 0, 1, 0);
		SetVecInt3 (vnCornerEnd, 1, 0, 1);
		//eStart = CORNER_BOTTOMLEFT;
		//eEnd = CORNER_TOPRIGHT;
		eStartDir = aeEdge[0][1][0];
		eEndDir = aeEdge[1][0][1];
		break;
	default:
		// Do nothing
		g_assert (FALSE);
		break;
	}

	nCornerIndex = ConvertToIndex (& vnCornerStart, & vnCornerSize);
	sStartColour = psColours->aCorner[nCornerIndex];
	nCornerIndex = ConvertToIndex (& vnCornerEnd, & vnCornerSize);
	sEndColour = psColours->aCorner[nCornerIndex];

	TileLine (pvPos, pvSize, & vnCornerStart, & vnCornerEnd, eStartDir, eEndDir, & sStartColour, & sEndColour, psRenderData);

	switch (eCentre) {
	case TILE_HORIZONTAL:
		SetVecInt3 (vnCornerStart, 0, 0, 1);
		SetVecInt3 (vnCornerEnd, 1, 0, 1);
		eStartDir = aeEdge[0][0][1];
		eEndDir = aeEdge[1][0][1];
		break;
	case TILE_VERTICAL:
		SetVecInt3 (vnCornerStart, 0, 0, 1);
		SetVecInt3 (vnCornerEnd, 0, 1, 1);
		eStartDir = aeEdge[0][0][1];
		eEndDir = aeEdge[0][1][1];
		break;
	case TILE_LONGITUDINAL:
		SetVecInt3 (vnCornerStart, 0, 1, 0);
		SetVecInt3 (vnCornerEnd, 0, 1, 1);
		eStartDir = aeEdge[0][1][0];
		eEndDir = aeEdge[0][1][1];
		break;
	case TILE_CROSS:
		SetVecInt3 (vnCornerStart, 1, 0, 0);
		SetVecInt3 (vnCornerEnd, 0, 1, 1);
		eStartDir = aeEdge[1][0][0];
		eEndDir = aeEdge[0][1][1];
		break;
	default:
		// Do nothing
		g_assert (FALSE);
		break;
	}

	nCornerIndex = ConvertToIndex (& vnCornerStart, & vnCornerSize);
	sStartColour = psColours->aCorner[nCornerIndex];
	nCornerIndex = ConvertToIndex (& vnCornerEnd, & vnCornerSize);
	sEndColour = psColours->aCorner[nCornerIndex];

	TileLine (pvPos, pvSize, & vnCornerStart, & vnCornerEnd, eStartDir, eEndDir, & sStartColour, & sEndColour, psRenderData);

	switch (eCentre) {
	case TILE_HORIZONTAL:
		SetVecInt3 (vnCornerStart, 0, 1, 1);
		SetVecInt3 (vnCornerEnd, 1, 1, 1);
		eStartDir = aeEdge[0][1][1];
		eEndDir = aeEdge[1][1][1];
		break;
	case TILE_VERTICAL:
		SetVecInt3 (vnCornerStart, 1, 0, 1);
		SetVecInt3 (vnCornerEnd, 1, 1, 1);
		eStartDir = aeEdge[1][0][1];
		eEndDir = aeEdge[1][1][1];
		break;
	case TILE_LONGITUDINAL:
		SetVecInt3 (vnCornerStart, 1, 1, 0);
		SetVecInt3 (vnCornerEnd, 1, 1, 1);
		eStartDir = aeEdge[1][1][0];
		eEndDir = aeEdge[1][1][1];
		break;
	case TILE_CROSS:
		SetVecInt3 (vnCornerStart, 1, 1, 0);
		SetVecInt3 (vnCornerEnd, 0, 0, 1);
		eStartDir = aeEdge[1][1][0];
		eEndDir = aeEdge[0][0][1];
		break;
	default:
		// Do nothing
		g_assert (FALSE);
		break;
	}

	nCornerIndex = ConvertToIndex (& vnCornerStart, & vnCornerSize);
	sStartColour = psColours->aCorner[nCornerIndex];
	nCornerIndex = ConvertToIndex (& vnCornerEnd, & vnCornerSize);
	sEndColour = psColours->aCorner[nCornerIndex];

	TileLine (pvPos, pvSize, & vnCornerStart, & vnCornerEnd, eStartDir, eEndDir, & sStartColour, & sEndColour, psRenderData);
}

float GetVolume3D (CelticPersist * psCelticData) {
	float fLength;
	float fCrossSection;
	float fVolume;

	fLength = psCelticData->psRenderData->fLength;
	fCrossSection = M_PI * pow (psCelticData->psRenderData->fThickness, 2.0);
	fVolume = fLength * fCrossSection;

	return fVolume;
}

void SaveSettingsCeltic3D (SettingsPersist * psSettingsData, CelticPersist * psCelticData) {
	SettingsPrintInt (psSettingsData, "Seed", psCelticData->uSeed);
	SettingsPrintInt (psSettingsData, "Width", psCelticData->vnSize.nX);
	SettingsPrintInt (psSettingsData, "Height", psCelticData->vnSize.nY);
	SettingsPrintInt (psSettingsData, "Depth", psCelticData->vnSize.nZ);
	SettingsPrintFloat (psSettingsData, "TileSizeX", psCelticData->vTileSize.fX);
	SettingsPrintFloat (psSettingsData, "TileSizeY", psCelticData->vTileSize.fY);
	SettingsPrintFloat (psSettingsData, "TileSizeZ", psCelticData->vTileSize.fZ);
	SettingsPrintFloat (psSettingsData, "Weirdness", psCelticData->fWeirdness);
	SettingsPrintInt (psSettingsData, "Orientation", psCelticData->eOrientation);
	SettingsPrintBool (psSettingsData, "Symmetrical", psCelticData->boSymmetrify);

	SettingsStartTag (psSettingsData, "render");
	SaveSettingsRender (psSettingsData, psCelticData->psRenderData);
	SettingsEndTag (psSettingsData, "render");
}

void LoadSettingsStartCeltic3D (SettingsPersist * psSettingsData, CelticPersist * psCelticData) {
	SettingsLoadParser * psLoadParser = NULL;

	psLoadParser = g_new0 (SettingsLoadParser, 1);

	psLoadParser->LoadProperty = CelticLoadProperty;
	psLoadParser->LoadSectionStart = CelticLoadSectionStart;
	psLoadParser->LoadSectionEnd = CelticLoadSectionEnd;
	psLoadParser->psData = psCelticData;
	AddParser (psLoadParser, psSettingsData);
}

void LoadSettingsEndCeltic3D (SettingsPersist * psSettingsData, CelticPersist * psCelticData) {
	SettingsLoadParser * psLoadParser = NULL;

	psLoadParser = GetParser (psSettingsData);
	g_free (psLoadParser);
	RemoveParser (psSettingsData);
}

static void CelticLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
	CelticPersist * psCelticData = (CelticPersist *)psData;

	switch (eType) {
	case SETTINGTYPE_BOOL:
		if (stricmp (szName, "Symmetrical") == 0) {
			psCelticData->boSymmetrify = *((bool*)(psValue));
		}
	case SETTINGTYPE_FLOAT:
		if (stricmp (szName, "TileSizeX") == 0) {
			SetTileX (*((float*)(psValue)), psCelticData);
		}
		else if (stricmp (szName, "TileSizeY") == 0) {
			SetTileY (*((float*)(psValue)), psCelticData);
		}
		else if (stricmp (szName, "TileSizeZ") == 0) {
			SetTileZ (*((float*)(psValue)), psCelticData);
		}
		else if (stricmp (szName, "Weirdness") == 0) {
			psCelticData->fWeirdness = *((float*)(psValue));
		}
		break;
	case SETTINGTYPE_INT:
		if (stricmp (szName, "Seed") == 0) {
			psCelticData->uSeed = *((int*)(psValue));
		}
		else if (stricmp (szName, "Width") == 0) {
			SetWidth (*((int*)(psValue)), psCelticData);
		}
		else if (stricmp (szName, "Height") == 0) {
			SetHeight (*((int*)(psValue)), psCelticData);
		}
		else if (stricmp (szName, "Depth") == 0) {
			SetDepth (*((int*)(psValue)), psCelticData);
		}
		else if (stricmp (szName, "Orientation") == 0) {
			psCelticData->eOrientation = *((int*)(psValue));
		}
		break;
	default:
		printf ("Unknown celtic property %s\n", szName);
		break;
	}
}

static void CelticLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {
	CelticPersist * psCelticData = (CelticPersist *)psData;
	// Any subsections go in here
	if (stricmp (szName, "render") == 0) {
		// Move in to the vis section
		LoadSettingsStartRender (psSettingsData, psCelticData->psRenderData);
	}
}

static void CelticLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
	CelticPersist * psCelticData = (CelticPersist *)psData;

	if (stricmp (szName, "celtic") == 0) {
		// Move out of the celtic section
		LoadSettingsEndCeltic (psSettingsData, psCelticData);
	}
}


