/*******************************************************************
 * Celtic2D.c
 * Generate celtic knot patterns
 *
 * David Llewellyn-Jones
 * http://www.flypig.co.uk
 *
 * July 2011
 *******************************************************************
*/

/* Includes */
#include "celtic.h"
#include "celtic2d.h"
#include "celtic_private.h"

#define _USE_MATH_DEFINES
#include <math.h>

/* Defines */
#define MAX_SIZE (1280)
#define X_SCALE (1.0f)
#define Y_SCALE (1.0f)
#define X_OFFSET ((3.0f * 9.0f) / 2.0f)
#define Y_OFFSET (-(3.0f * 9.0f) / 2.0f)
#define Z_PLANE (0.0f)
#define WEAVE_HEIGHT (0.2f)
#define CONTROL_SCALE (0.4f)
#define BEZIERS_PER_TILE (4)

/* Enums */

/* Structures */

/* Function prototypes */

/* Virtual function prototypes */
void DeleteCelticPersist2D (CelticPersist * psCelticData);
void GenerateKnot2D (CelticPersist * psCelticData);
void RenderKnots2D (CelticPersist * psCelticData);

/* Characteristics */
float GetVolume2D (CelticPersist * psCelticData);

/* Settings loading/saving */
void SaveSettingsCeltic2D (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
void LoadSettingsStartCeltic2D (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
void LoadSettingsEndCeltic2D (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
bool ExportModel2D (char const * szFilename, bool boBinary, CelticPersist * psCelticData);

/* Local function prototypes */
static void ImageBezierSplit (float fX1, float fY1, float fXDir1, float fYDir1, float fX2, float fY2, float fXDir2, float fYDir2, float fRatio, float fHStart, float fHMid, float fHEnd, ColFloats const * psStartColour, ColFloats const * psEndColour, RenderPersist * psRenderData);
static void TileLine (float fX, float fY, float fWidth, float fHeight, CORNER eStart, CORNER eEnd, TILE eStartDir, TILE eEndDir, ColFloats const * psStartColour, ColFloats const * psEndColour, RenderPersist * psRenderData);
static void TileKnot (float fX, float fY, float fWidth, float fHeight, TILE eTopLeft, TILE eTopRight, TILE eBottomRight, TILE eBottomLeft, TILE eCentre, Cube2Colour const * psColours, RenderPersist * psRenderData);
static TILE GetCorner (int nXPos, int nYPos, CORNER eCorner, CelticPersist * psCelticData);
static void SetCorner (int nXPos, int nYPos, CORNER eCorner, TILE eValue, CelticPersist * psCelticData);
static TILE GetCentre (int nXPos, int nYPos, CelticPersist * psCelticData);
static void SetCentre (int nXPos, int nYPos, TILE eValue, CelticPersist * psCelticData);
static void PopulateTiles (TILE eOrientation, CelticPersist * psCelticData);
static void SymmetrifyTiles (CelticPersist * psCelticData);
static void FollowKnotInside (VecInt3 * pvnPos, VecInt3 * pvnCorner, CelticPersist * psCelticData);
static void FollowKnotToNext (VecInt3 * pvnPos, VecInt3 * pvnCorner, CelticPersist * psCelticData);
static void ColourLoop (VecInt3 * pvnPos, VecInt3 * pvnCorner, Cube2Complete * asCompleted, ColFloats const * psColour, CelticPersist * psCelticData);
static int ColourTiles (CelticPersist * psCelticData);
static void SelectColour (ColFloats * psColour, RenderPersist * psRenderData);
static void CelticLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
static void CelticLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
static void CelticLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);

/* Function defininitions */
CelticPersist * NewCelticPersist2D (int nWidth, int nHeight, float fTileX, float fTileY, BezPersist * psBezData) {
	CelticPersist * psCelticData;

	psCelticData = (CelticPersist *)calloc (1, sizeof (CelticPersist));
	psCelticData->uSeed = 0u;
	psCelticData->vnSize.nX = nWidth;
	psCelticData->vnSize.nY = nHeight;
	psCelticData->vnSize.nZ = 1;
	psCelticData->vTileSize.fX = fTileX;
	psCelticData->vTileSize.fY = fTileY;
	psCelticData->vTileSize.fZ = 0.0f;
	psCelticData->fWeirdness = 0.2f;
	psCelticData->eOrientation = TILE_HORIZONTAL;
	psCelticData->boSymmetrify = TRUE;
	psCelticData->boDebug = FALSE;
	psCelticData->psRenderData = NewRenderPersist (-(((float)nWidth) * fTileX / 2.0f), -(((float)nHeight) * fTileY / 2.0f), 0.0f);
	psCelticData->psRenderData->psBezData = psBezData;
	
	psCelticData->aeCorner = NULL;
	psCelticData->aeCentre = NULL;

	/* Set up the virtual functions */
	psCelticData->DeleteCelticPersist = & DeleteCelticPersist2D;
	
	psCelticData->GenerateKnot = & GenerateKnot2D;
	psCelticData->RenderKnots = & RenderKnots2D;

	psCelticData->GetVolume = & GetVolume2D;

	psCelticData->SaveSettingsCeltic = & SaveSettingsCeltic2D;
	psCelticData->LoadSettingsStartCeltic =  LoadSettingsStartCeltic2D;
	psCelticData->LoadSettingsEndCeltic =  LoadSettingsEndCeltic2D;
	psCelticData->ExportModel = & ExportModel2D;

	return psCelticData;
}

void DeleteCelticPersist2D (CelticPersist * psCelticData) {
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
void GenerateKnot2D (CelticPersist * psCelticData) {
	int nBezier;
	int nBeziers;

	/* Re-seed the random number generator */
	srand (psCelticData->uSeed);

	if (psCelticData->aeCorner) {
		free (psCelticData->aeCorner);
	}
	if (psCelticData->aeCentre) {
		free (psCelticData->aeCentre);
	}
	psCelticData->aeCorner = (TILE *)calloc (((psCelticData->vnSize.nY + 1) * (psCelticData->vnSize.nX + 1)), sizeof (TILE));
	psCelticData->aeCentre = (TILE *)calloc (((psCelticData->vnSize.nY) * (psCelticData->vnSize.nX)), sizeof (TILE));

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
	nBeziers = (psCelticData->vnSize.nX * psCelticData->vnSize.nY * BEZIERS_PER_TILE);
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
		if ((pvnPos->nX >= 0) && (pvnPos->nY >= 0) && (pvnPos->nX < psCelticData->vnSize.nX) && (pvnPos->nY < psCelticData->vnSize.nY)) {
			eCentre = GetCentre (pvnPos->nX, pvnPos->nY, psCelticData);

			switch (eCentre) {
			case TILE_HORIZONTAL:
				pvnCorner->nX = 1 - pvnCorner->nX;
				break;
			case TILE_VERTICAL:
				pvnCorner->nY = 1 - pvnCorner->nY;
				break;
				break;
			case TILE_CROSS:
				pvnCorner->nX = 1 - pvnCorner->nX;
				pvnCorner->nY = 1 - pvnCorner->nY;
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
	CORNER eCornerIndex;
	VecInt3 vnSize;

	SetVecInt3 (vnSize, 2, 2, 1)

	if (pvnPos && pvnCorner) {
		if ((pvnPos->nX >= 0) && (pvnPos->nY >= 0) && (pvnPos->nX < psCelticData->vnSize.nX) && (pvnPos->nY < psCelticData->vnSize.nY)) {
			eCornerIndex = (CORNER)ConvertToIndex (pvnCorner, &vnSize);
			eCorner = GetCorner (pvnPos->nX, pvnPos->nY, eCornerIndex, psCelticData);
			switch (eCorner) {
				case TILE_HORIZONTAL:
					pvnPos->nX += ((pvnCorner->nX > 0) ? 1 : -1);
					pvnCorner->nX = (1 - pvnCorner->nX);
					break;
				case TILE_VERTICAL:
					pvnPos->nY += ((pvnCorner->nY > 0) ? 1 : -1);
					pvnCorner->nY = (1 - pvnCorner->nY);
					break;
				case TILE_CROSS:
					pvnPos->nX += ((pvnCorner->nX > 0) ? 1 : -1);
					pvnCorner->nX = (1 - pvnCorner->nX);
					pvnPos->nY += ((pvnCorner->nY > 0) ? 1 : -1);
					pvnCorner->nY = (1 - pvnCorner->nY);
					break;
				default:
					// Do nothing
					break;
			}
		}
	}
}

static void ColourLoop (VecInt3 * pvnPos, VecInt3 * pvnCorner, Cube2Complete * asCompleted, ColFloats const * psColour, CelticPersist * psCelticData) {
	int nIndex;
	int nCornerIndex;
	VecInt3 vnUnitSize;

	SetVecInt3 (vnUnitSize, 2, 2, 1);

	nIndex = ConvertToIndex (pvnPos, & psCelticData->vnSize);
	nCornerIndex = ConvertToIndex (pvnCorner, & vnUnitSize);

	do {
		asCompleted[nIndex].aCorner[nCornerIndex] = TRUE;
		FollowKnotInside (pvnPos, pvnCorner, psCelticData);

		nIndex = ConvertToIndex (pvnPos, & psCelticData->vnSize);
		nCornerIndex = ConvertToIndex (pvnCorner, & vnUnitSize);
		psCelticData->psRenderData->asColour2D[nIndex].aCorner[nCornerIndex] = * psColour;
		asCompleted[nIndex].aCorner[nCornerIndex] = TRUE;

		FollowKnotToNext (pvnPos, pvnCorner, psCelticData);

		nIndex = ConvertToIndex (pvnPos, & psCelticData->vnSize);
		nCornerIndex = ConvertToIndex (pvnCorner, & vnUnitSize);
		psCelticData->psRenderData->asColour2D[nIndex].aCorner[nCornerIndex] = * psColour;
	} while (asCompleted[nIndex].aCorner[nCornerIndex] != TRUE);
}

/* Define a colour for each of the pieces of the string */
static int ColourTiles (CelticPersist * psCelticData) {
	VecInt3 vnPos;
	Cube2Complete * asCompleted;
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

	if (psCelticData->psRenderData->asColour2D) {
		free (psCelticData->psRenderData->asColour2D);
	}
	nTotal = (psCelticData->vnSize.nY * psCelticData->vnSize.nX);
	psCelticData->psRenderData->asColour2D = (Cube2Colour *)malloc ((nTotal * sizeof (Cube2Colour)));

	asCompleted = (Cube2Complete *)malloc (nTotal * sizeof (Cube2Complete));

	for (nCount = 0; nCount < nTotal; nCount++) {
		for (nCorner = 0; nCorner < 4; nCorner++) {
			asCompleted[nCount].aCorner[nCorner] = FALSE;
			psCelticData->psRenderData->asColour2D[nCount].aCorner[nCorner].fRed = 0.0f;
			psCelticData->psRenderData->asColour2D[nCount].aCorner[nCorner].fGreen = 0.0f;
			psCelticData->psRenderData->asColour2D[nCount].aCorner[nCorner].fBlue = 0.0f;
		}
	}

	nLoops = 0;
	vnPos.nZ = 0;
	for (vnPos.nX = 0; vnPos.nX < psCelticData->vnSize.nX; vnPos.nX++) {
		for (vnPos.nY = 0; vnPos.nY < psCelticData->vnSize.nY; vnPos.nY++) {
			for (nCorner = 0; nCorner < 4; nCorner++) {
				nIndex = ConvertToIndex (& vnPos, & psCelticData->vnSize);
				if (asCompleted[nIndex].aCorner[nCorner] == FALSE) {
					vnLoopPos = vnPos;
					SetVecInt3 (vnLoopCorner, (nCorner % 2), ((nCorner / 2) % 2), (nCorner / 4));
					eCorner = GetCorner (vnLoopPos.nX, vnLoopPos.nY, (CORNER)nCorner, psCelticData);
					SelectColour (& sColour, psCelticData->psRenderData);
					if (eCorner != TILE_INVALID) {
						ColourLoop (& vnLoopPos, & vnLoopCorner, asCompleted, & sColour, psCelticData);
						nLoops++;
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
static TILE GetCorner (int nXPos, int nYPos, CORNER eCorner, CelticPersist * psCelticData) {
	int nXIndex;
	int nYIndex;
	TILE eReturn = TILE_INVALID;

	nXIndex = nXPos;
	nYIndex = nYPos;

	nXIndex += ((eCorner == CORNER_BOTTOMRIGHT) || (eCorner == CORNER_TOPRIGHT));
	nYIndex += ((eCorner == CORNER_BOTTOMLEFT) || (eCorner == CORNER_BOTTOMRIGHT));

	if (psCelticData && psCelticData->aeCorner && (nXPos >= 0) && (nXPos < psCelticData->vnSize.nX) && (nYPos >= 0) && (nYPos < psCelticData->vnSize.nY)) {
		eReturn = psCelticData->aeCorner[(nYIndex * (psCelticData->vnSize.nX + 1)) + nXIndex];
	}

	return eReturn;
}

/* Set the tile direction for a given corner */
static void SetCorner (int nXPos, int nYPos, CORNER eCorner, TILE eValue, CelticPersist * psCelticData) {
	int nXIndex;
	int nYIndex;

	nXIndex = nXPos;
	nYIndex = nYPos;

	nXIndex += ((eCorner == CORNER_BOTTOMRIGHT) || (eCorner == CORNER_TOPRIGHT));
	nYIndex += ((eCorner == CORNER_BOTTOMLEFT) || (eCorner == CORNER_BOTTOMRIGHT));

	if (psCelticData && psCelticData->aeCorner && (nXPos >= 0) && (nXPos < psCelticData->vnSize.nX) && (nYPos >= 0) && (nYPos < psCelticData->vnSize.nY)) {
		psCelticData->aeCorner[(nYIndex * (psCelticData->vnSize.nX + 1)) + nXIndex] = eValue;
	}
}

/* Get the tile direction for the centre of a given tile */
static TILE GetCentre (int nXPos, int nYPos, CelticPersist * psCelticData) {
	TILE eReturn = TILE_INVALID;

	if (psCelticData && psCelticData->aeCentre && (nXPos >= 0) && (nXPos < psCelticData->vnSize.nX) && (nYPos >= 0) && (nYPos < psCelticData->vnSize.nY)) {
		eReturn = psCelticData->aeCentre[(nYPos * psCelticData->vnSize.nX) + nXPos];
	}

	return eReturn;
}

/* Set the tile direction for the centre of a given tile */
static void SetCentre (int nXPos, int nYPos, TILE eValue, CelticPersist * psCelticData) {
	if (psCelticData && psCelticData->aeCentre && (nXPos >= 0) && (nXPos < psCelticData->vnSize.nX) && (nYPos >= 0) && (nYPos < psCelticData->vnSize.nY)) {
		psCelticData->aeCentre[(nYPos * psCelticData->vnSize.nX) + nXPos] = eValue;
	}
}

/* Poplulate the tiles with a random set of directions */
static void PopulateTiles (TILE eOrientation, CelticPersist * psCelticData) {
	int nXPos;
	int nYPos;
	int nCorner;
	TILE eTileChoice;
	float fCrossRand;

	/* Populate the edge and corner arrays */
	for (nXPos = 0; nXPos < psCelticData->vnSize.nX; nXPos++) {
		for (nYPos = 0; nYPos < psCelticData->vnSize.nY; nYPos++) {
			for (nCorner = 0; nCorner < (int)CORNER_NUM; nCorner++) {
				eTileChoice = (TILE)(rand () % ((int)TILE_NUM - 1));
				SetCorner (nXPos, nYPos, (CORNER)nCorner, eTileChoice, psCelticData);
			}

			fCrossRand = ((float)rand () / (float)RAND_MAX);
			eTileChoice = (TILE)((rand () % ((int)TILE_NUM - 2)) + 1);
			if (fCrossRand >= psCelticData->fWeirdness) {
				eTileChoice = TILE_CROSS;
			}
			SetCentre (nXPos, nYPos, eTileChoice, psCelticData);
		}
	}

	/* Deal with the horizontal edges */
	eTileChoice = (eOrientation == TILE_VERTICAL ? TILE_HORIZONTAL : TILE_INVALID);
	for (nXPos = 0; nXPos < psCelticData->vnSize.nX; nXPos++) {
		SetCorner (nXPos, 0, CORNER_TOPLEFT, eTileChoice, psCelticData);
		SetCorner (nXPos, 0, CORNER_TOPRIGHT, eTileChoice, psCelticData);
		SetCorner (nXPos, (psCelticData->vnSize.nY - 1), CORNER_BOTTOMLEFT, eTileChoice, psCelticData);
		SetCorner (nXPos, (psCelticData->vnSize.nY - 1), CORNER_BOTTOMRIGHT, eTileChoice, psCelticData);
	}

	/* Deal with the vertical edges */
	eTileChoice = (eOrientation == TILE_HORIZONTAL ? TILE_VERTICAL : TILE_INVALID);
	for (nYPos = 0; nYPos < psCelticData->vnSize.nY; nYPos++) {
		SetCorner (0, nYPos, CORNER_TOPLEFT, eTileChoice, psCelticData);
		SetCorner (0, nYPos, CORNER_BOTTOMLEFT, eTileChoice, psCelticData);
		SetCorner ((psCelticData->vnSize.nX - 1), nYPos, CORNER_TOPRIGHT, eTileChoice, psCelticData);
		SetCorner ((psCelticData->vnSize.nX - 1), nYPos, CORNER_BOTTOMRIGHT, eTileChoice, psCelticData);
	}


	switch (eOrientation) {
	case TILE_VERTICAL:
		/* Deal with the edge centres */
		for (nYPos = 0; nYPos < psCelticData->vnSize.nY; nYPos++) {
			SetCentre (0, nYPos, TILE_VERTICAL, psCelticData);
			SetCentre ((psCelticData->vnSize.nX - 1), nYPos, TILE_VERTICAL, psCelticData);
		}
		/* Deal with the internal vertical edges */
		for (nYPos = 1; nYPos < (psCelticData->vnSize.nY - 1); nYPos++) {
			eTileChoice = (TILE)(rand () % 2);
			SetCorner (0, nYPos, CORNER_TOPRIGHT, eTileChoice, psCelticData);
			eTileChoice = (TILE)(rand () % 2);
			SetCorner (0, nYPos, CORNER_BOTTOMRIGHT, eTileChoice, psCelticData);
			eTileChoice = (TILE)(rand () % 2);
			SetCorner ((psCelticData->vnSize.nX - 1), nYPos, CORNER_TOPLEFT, eTileChoice, psCelticData);
			eTileChoice = (TILE)(rand () % 2);
			SetCorner ((psCelticData->vnSize.nX - 1), nYPos, CORNER_BOTTOMLEFT, eTileChoice, psCelticData);
		}
		break;
	case TILE_HORIZONTAL:
		/* Deal with the edge centres */
		for (nXPos = 0; nXPos < psCelticData->vnSize.nX; nXPos++) {
			SetCentre (nXPos, 0, TILE_HORIZONTAL, psCelticData);
			SetCentre (nXPos, (psCelticData->vnSize.nY - 1), TILE_HORIZONTAL, psCelticData);
		}
		/* Deal with the internal horizontal edges */
		for (nXPos = 1; nXPos < (psCelticData->vnSize.nX - 1); nXPos++) {
			eTileChoice = (TILE)(2 * (rand () % 2));
			SetCorner (nXPos, 0, CORNER_BOTTOMLEFT, eTileChoice, psCelticData);
			eTileChoice = (TILE)(2 * (rand () % 2));
			SetCorner (nXPos, 0, CORNER_BOTTOMRIGHT, eTileChoice, psCelticData);
			eTileChoice = (TILE)(2 * (rand () % 2));
			SetCorner (nXPos, (psCelticData->vnSize.nY - 1), CORNER_TOPLEFT, eTileChoice, psCelticData);
			eTileChoice = (TILE)(2 * (rand () % 2));
			SetCorner (nXPos, (psCelticData->vnSize.nY - 1), CORNER_TOPRIGHT, eTileChoice, psCelticData);
		}
		break;
	default:
		/* Do nothing */
		break;
	}

	/* Canvas Corners */
	SetCorner (0, 0, CORNER_TOPLEFT, TILE_INVALID, psCelticData);
	SetCorner (0, (psCelticData->vnSize.nY - 1), CORNER_BOTTOMLEFT, TILE_INVALID, psCelticData);
	SetCorner ((psCelticData->vnSize.nX - 1), 0, CORNER_TOPRIGHT, TILE_INVALID, psCelticData);
	SetCorner ((psCelticData->vnSize.nX - 1), (psCelticData->vnSize.nY - 1), CORNER_BOTTOMRIGHT, TILE_INVALID, psCelticData);
}

/* Alter the tile directions to create a rotationally symmetric knot */
static void SymmetrifyTiles (CelticPersist * psCelticData) {
	int nXPos = 0;
	int nYPos = 0;
	TILE eTileChoice;
	TILE eTileCentre;
	int nMiddleX;
	int nMiddleY;

	/* Deal with symmetry */
	nMiddleY = (psCelticData->vnSize.nY / 2);
	for (nXPos = 0; nXPos < psCelticData->vnSize.nX; nXPos++) {
		for (nYPos = 0; nYPos < nMiddleY; nYPos++) {
			eTileCentre = GetCentre (nXPos, nYPos, psCelticData);
			SetCentre (psCelticData->vnSize.nX - nXPos - 1, psCelticData->vnSize.nY - nYPos - 1, eTileCentre, psCelticData);

			eTileChoice = GetCorner (nXPos, nYPos, CORNER_TOPLEFT, psCelticData);
			SetCorner (psCelticData->vnSize.nX - nXPos - 1, psCelticData->vnSize.nY - nYPos - 1, CORNER_BOTTOMRIGHT, eTileChoice, psCelticData);

			eTileChoice = GetCorner (nXPos, nYPos, CORNER_TOPRIGHT, psCelticData);
			SetCorner (psCelticData->vnSize.nX - nXPos - 1, psCelticData->vnSize.nY - nYPos - 1, CORNER_BOTTOMLEFT, eTileChoice, psCelticData);

			eTileChoice = GetCorner (nXPos, nYPos, CORNER_BOTTOMLEFT, psCelticData);
			SetCorner (psCelticData->vnSize.nX - nXPos - 1, psCelticData->vnSize.nY - nYPos - 1, CORNER_TOPRIGHT, eTileChoice, psCelticData);

			eTileChoice = GetCorner (nXPos, nYPos, CORNER_BOTTOMRIGHT, psCelticData);
			SetCorner (psCelticData->vnSize.nX - nXPos - 1, psCelticData->vnSize.nY - nYPos - 1, CORNER_TOPLEFT, eTileChoice, psCelticData);
		}
	}

	// If there's an odd number of tiles, deal with the central line
	if ((psCelticData->vnSize.nY % 2) == 1) {
		nMiddleX = (psCelticData->vnSize.nX / 2);
		nMiddleY = (psCelticData->vnSize.nY / 2);
		for (nXPos = 0; nXPos < nMiddleX; nXPos++) {
			eTileCentre = GetCentre (nXPos, nYPos, psCelticData);
			SetCentre (psCelticData->vnSize.nX - nXPos - 1, nYPos, eTileCentre, psCelticData);
		}
	}
}

/* Render the knots to the canvas */
void RenderKnots2D (CelticPersist * psCelticData) {
	int nXPos;
	int nYPos;
	int nCorner;
	TILE eTileCentre;
	TILE eTileEdge[CORNER_NUM];
	VecInt3 vnPos;
	Cube2Colour sColours;
	int nIndex;

	/* Sanity check */
	g_assert ((psCelticData->psRenderData->nBezierNum == (psCelticData->vnSize.nX * psCelticData->vnSize.nY * BEZIERS_PER_TILE)));
	g_assert (psCelticData->psRenderData->psBezierStart);
	psCelticData->psRenderData->psBezierCurrent = psCelticData->psRenderData->psBezierStart;

	// Reset the length for volume calculation
	psCelticData->psRenderData->fLength = 0.0f;

	// Clear the currently stored beziers if there are any
	StoreBeziers (FALSE, psCelticData->psRenderData->psBezData);

	// Set the next set of beziers to be stored
	StoreBeziers (TRUE, psCelticData->psRenderData->psBezData);

	/* Draw the knots */
	for (nXPos = 0; nXPos < psCelticData->vnSize.nX; nXPos++) {
		for (nYPos = 0; nYPos < psCelticData->vnSize.nY; nYPos++) {
			SetVecInt3 (vnPos, nXPos, nYPos, 0);
			for (nCorner = 0; nCorner < (int)CORNER_NUM; nCorner++) {
				eTileEdge[nCorner] = GetCorner (nXPos, nYPos, (CORNER)nCorner, psCelticData);
			}
			nIndex = ConvertToIndex (& vnPos, & psCelticData->vnSize);
			sColours = psCelticData->psRenderData->asColour2D[nIndex];

			eTileCentre = GetCentre (nXPos, nYPos, psCelticData);
			TileKnot ((nXPos * psCelticData->vTileSize.fX), (nYPos * psCelticData->vTileSize.fY), psCelticData->vTileSize.fX, psCelticData->vTileSize.fY, eTileEdge[(int)CORNER_TOPLEFT], eTileEdge[(int)CORNER_TOPRIGHT], eTileEdge[(int)CORNER_BOTTOMRIGHT], eTileEdge[(int)CORNER_BOTTOMLEFT], eTileCentre, & sColours, psCelticData->psRenderData);
		}
	}
}

/* Render a bezier curve */
/* We now use ImageBezierSplit instead */
/*
static void ImageBezier (float fX1, float fY1, float fXDir1, float fYDir1, float fX2, float fY2, float fXDir2, float fYDir2, RenderPersist * psRenderData) {
	Vector3 vStart;
	Vector3 vStartDir;
	Vector3 vEnd;
	Vector3 vEndDir;

	vStart.fX = (fX1 / X_SCALE) + psRenderData->vOffset.fX;
	vStart.fY = (fY1 / Y_SCALE) + psRenderData->vOffset.fY;
	vStart.fZ = Z_PLANE;

	vStartDir.fX = (fXDir1 / X_SCALE) + psRenderData->vOffset.fX;
	vStartDir.fY = (fYDir1 / Y_SCALE) + psRenderData->vOffset.fY;
	vStartDir.fZ = Z_PLANE;

	vEnd.fX = (fX2 / X_SCALE) + psRenderData->vOffset.fX;
	vEnd.fY = (fY2 / Y_SCALE) + psRenderData->vOffset.fY;
	vEnd.fZ = Z_PLANE;

	vEndDir.fX = (fXDir2 / X_SCALE) + psRenderData->vOffset.fX;
	vEndDir.fY = (fYDir2 / Y_SCALE) + psRenderData->vOffset.fY;
	vEndDir.fZ = Z_PLANE;

	SetBezierControlPoints (psRenderData->fThickness, vStart, vStartDir, vEnd, vEndDir, NULL, NULL, psRenderData->psBezierCurrent, psRenderData->psBezData);
	psRenderData->psBezierCurrent = GetBezierNext (psRenderData->psBezierCurrent);
}
*/

/* Render a bezier curve pushed forwards or backwards in the middle */
static void ImageBezierSplit (float fX1, float fY1, float fXDir1, float fYDir1, float fX2, float fY2, float fXDir2, float fYDir2, float fRatio, float fHStart, float fHMid, float fHEnd, ColFloats const * psStartColour, ColFloats const * psEndColour, RenderPersist * psRenderData) {
	Vector3 vStart;
	Vector3 vStartDir;
	Vector3 vEnd;
	Vector3 vEndDir;
	Vector3 vMid;
	Vector3 vMidDirBack;
	Vector3 vMidDirForward;
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

	vStart.fX = (fX1 / X_SCALE) + psRenderData->vOffset.fX;
	vStart.fY = (fY1 / Y_SCALE) + psRenderData->vOffset.fY;
	vStart.fZ = Z_PLANE;

	vStartDir.fX = (fXDir1 / X_SCALE) + psRenderData->vOffset.fX;
	vStartDir.fY = (fYDir1 / Y_SCALE) + psRenderData->vOffset.fY;
	vStartDir.fZ = Z_PLANE;

	vEnd.fX = (fX2 / X_SCALE) + psRenderData->vOffset.fX;
	vEnd.fY = (fY2 / Y_SCALE) + psRenderData->vOffset.fY;
	vEnd.fZ = Z_PLANE;

	vEndDir.fX = (fXDir2 / X_SCALE) + psRenderData->vOffset.fX;
	vEndDir.fY = (fYDir2 / Y_SCALE) + psRenderData->vOffset.fY;
	vEndDir.fZ = Z_PLANE;

	SplitBezier (vStart, & vStartDir, vEnd, & vEndDir, fRatio, & vMid, & vMidDirBack, & vMidDirForward);

	vStart.fZ += fHStart;
	vStartDir.fZ += fHStart;
	vEnd.fZ += fHEnd;
	vEndDir.fZ += fHEnd;
	vMid.fZ += fHMid;
	vMidDirBack.fZ += fHMid;
	vMidDirForward.fZ += fHMid;

	g_assert (psRenderData->psBezierCurrent);
	SetBezierControlPoints (psRenderData->fThickness, vStart, vStartDir, vMid, vMidDirBack, (float const *)psStartColour, (float const *)(& sMidColour), psRenderData->psBezierCurrent, psRenderData->psBezData);
	psRenderData->psBezierCurrent = GetBezierNext (psRenderData->psBezierCurrent);

	g_assert (psRenderData->psBezierCurrent);
	SetBezierControlPoints (psRenderData->fThickness, vMid, vMidDirForward, vEnd, vEndDir, (float const *)psMidColour, (float const *)psEndColour, psRenderData->psBezierCurrent, psRenderData->psBezData);
	psRenderData->psBezierCurrent = GetBezierNext (psRenderData->psBezierCurrent);
}

/* Render a single bezier curve given the parameters of a tile */
static void TileLine (float fX, float fY, float fWidth, float fHeight, CORNER eStart, CORNER eEnd, TILE eStartDir, TILE eEndDir, ColFloats const * psStartColour, ColFloats const * psEndColour, RenderPersist * psRenderData) {
	float fX1;
	float fY1;
	float fXDir1;
	float fYDir1;
	float fX2;
	float fY2;
	float fXDir2;
	float fYDir2;
	float fXShift1;
	float fYShift1;
	float fXShift2;
	float fYShift2;
	float fHStart;
	float fHMid;
	float fHEnd;
	Vector3 vPos[2];
	Vector3 vDir[2];
	bool boDraw = TRUE;

	fX1 = 0.0f;
	fY1 = 0.0f;
	fXDir1 = 0.0f;
	fYDir1 = 0.0f;
	fX2 = 0.0f;
	fY2 = 0.0f;
	fXDir2 = 0.0f;
	fYDir2 = 0.0f;
	fXShift1 = 0.0f;
	fYShift1 = 0.0f;
	fXShift2 = 0.0f;
	fYShift2 = 0.0f;
	fHStart = 0.0f;
	fHMid = 0.0f;
	fHEnd = 0.0f;

	switch (eStartDir) {
	case TILE_HORIZONTAL:
		fXDir1 = (fWidth * CONTROL_SCALE);
		fYDir1 = 0;
		fXShift1 = 0;
		fYShift1 = psRenderData->vLineInset.fY;
		break;
	case TILE_VERTICAL:
		fXDir1 = 0;
		fYDir1 = (fHeight * CONTROL_SCALE);
		fXShift1 = psRenderData->vLineInset.fX;
		fYShift1 = 0;
		break;
	case TILE_CROSS:
		fXDir1 = (fWidth * CONTROL_SCALE);
		fYDir1 = (fHeight * CONTROL_SCALE);
		fXShift1 = 0;
		fYShift1 = 0;
		fHStart = psRenderData->fWeaveHeight;
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
		fXDir2 = (fWidth * CONTROL_SCALE);
		fYDir2 = 0;
		fXShift2 = 0;
		fYShift2 = psRenderData->vLineInset.fY;
		break;
	case TILE_VERTICAL:
		fXDir2 = 0;
		fYDir2 = (fHeight * CONTROL_SCALE);
		fXShift2 = psRenderData->vLineInset.fX;
		fYShift2 = 0;
		break;
	case TILE_CROSS:
		fXDir2 = (fWidth * CONTROL_SCALE);
		fYDir2 = (fHeight * CONTROL_SCALE);
		fXShift2 = 0;
		fYShift2 = 0;
		fHEnd = psRenderData->fWeaveHeight;
		break;
	case TILE_INVALID:
		boDraw = FALSE;
		break;
	default:
		// Do nothing
		g_assert (FALSE);
		break;
	}

	if (boDraw) {
		switch (eStart) {
		case CORNER_TOPLEFT:
			fX1 = fX + fXShift1;
			fY1 = fY + fYShift1;
			fXDir1 = fX1 + fXDir1;
			fYDir1 = fY1 + fYDir1;
			break;
		case CORNER_TOPRIGHT:
			fX1 = fX + fWidth - fXShift1;
			fY1 = fY + fYShift1;
			fXDir1 = fX1 - fXDir1;
			fYDir1 = fY1 + fYDir1;
			fHStart = -fHStart;
			break;
		case CORNER_BOTTOMRIGHT:
			fX1 = fX + fWidth - fXShift1;
			fY1 = fY + fHeight - fYShift1;
			fXDir1 = fX1 - fXDir1;
			fYDir1 = fY1 - fYDir1;
			break;
		case CORNER_BOTTOMLEFT:
			fX1 = fX + fXShift1;
			fY1 = fY + fHeight - fYShift1;
			fXDir1 = fX1 + fXDir1;
			fYDir1 = fY1 - fYDir1;
			fHStart = -fHStart;
			break;
		default:
			// Do nothing
			break;
		}

		switch (eEnd) {
		case CORNER_TOPLEFT:
			fX2 = fX + fXShift2;
			fY2 = fY + fYShift2;
			fXDir2 = fX2 + fXDir2;
			fYDir2 = fY2 + fYDir2;
			break;
		case CORNER_TOPRIGHT:
			fX2 = fX + fWidth - fXShift2;
			fY2 = fY + fYShift2;
			fXDir2 = fX2 - fXDir2;
			fYDir2 = fY2 + fYDir2;
			fHEnd = -fHEnd;
			break;
		case CORNER_BOTTOMRIGHT:
			fX2 = fX + fWidth - fXShift2;
			fY2 = fY + fHeight - fYShift2;
			fXDir2 = fX2 - fXDir2;
			fYDir2 = fY2 - fYDir2;
			break;
		case CORNER_BOTTOMLEFT:
			fX2 = fX + fXShift2;
			fY2 = fY + fHeight - fYShift2;
			fXDir2 = fX2 + fXDir2;
			fYDir2 = fY2 - fYDir2;
			fHEnd = -fHEnd;
			break;
		default:
			// Do nothing
			break;
		}

		if (((eStart == CORNER_TOPLEFT) && (eEnd == CORNER_BOTTOMRIGHT))
			|| ((eStart == CORNER_BOTTOMRIGHT) && (eEnd == CORNER_TOPLEFT))) {
			fHMid = -psRenderData->fWeaveHeight;
		}
		if (((eStart == CORNER_TOPRIGHT) && (eEnd == CORNER_BOTTOMLEFT))
			|| ((eStart == CORNER_BOTTOMLEFT) && (eEnd == CORNER_TOPRIGHT))) {
			fHMid = psRenderData->fWeaveHeight;
		}

		ImageBezierSplit (fX1, fY1, fXDir1, fYDir1, fX2, fY2, fXDir2, fYDir2, 0.5f, fHStart, fHMid, fHEnd, psStartColour, psEndColour, psRenderData);

		SetVector3 (vPos[0], fX1, fY1, 0.0f);
		SetVector3 (vDir[0], fXDir1, fYDir1, 0.0f);
		SetVector3 (vPos[1], fX2, fY2, 0.0f);
		SetVector3 (vDir[1], fXDir2, fYDir2, 0.0f);
		psRenderData->fLength += BezierCalculateLength (vPos[0], vDir[0], vPos[1], vDir[1], psRenderData->psBezData);
	}
	else {
		ImageBezierSplit (0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, NULL, NULL, psRenderData);
	}
}

/* Render a tile */
static void TileKnot (float fX, float fY, float fWidth, float fHeight, TILE eTopLeft, TILE eTopRight, TILE eBottomRight, TILE eBottomLeft, TILE eCentre, Cube2Colour const * psColours, RenderPersist * psRenderData) {
	CORNER eStart;
	CORNER eEnd;
	TILE eStartDir;
	TILE eEndDir;
	ColFloats sStartColour;
	ColFloats sEndColour;

	eEnd = CORNER_BOTTOMRIGHT;
	eEndDir = eBottomRight;

	eStart = CORNER_TOPLEFT;
	eStartDir = eTopLeft;

	switch (eCentre) {
	case TILE_HORIZONTAL:
		eEnd = CORNER_TOPRIGHT;
		eEndDir = eTopRight;
		break;
	case TILE_VERTICAL:
		eEnd = CORNER_BOTTOMLEFT;
		eEndDir = eBottomLeft;
		break;
	case TILE_CROSS:
		eEnd = CORNER_BOTTOMRIGHT;
		eEndDir = eBottomRight;
		break;
	default:
		// Do nothing
		break;
	}

	sStartColour = psColours->aCorner[(int)eStart];
	sEndColour = psColours->aCorner[(int)eEnd];

	TileLine (fX, fY, fWidth, fHeight, eStart, eEnd, eStartDir, eEndDir, & sStartColour, & sEndColour, psRenderData);

	switch (eCentre) {
	case TILE_HORIZONTAL:
		eStart = CORNER_BOTTOMLEFT;
		eEnd = CORNER_BOTTOMRIGHT;
		eStartDir = eBottomLeft;
		eEndDir = eBottomRight;
		break;
	case TILE_VERTICAL:
		eStart = CORNER_TOPRIGHT;
		eEnd = CORNER_BOTTOMRIGHT;
		eStartDir = eTopRight;
		eEndDir = eBottomRight;
		break;
	case TILE_CROSS:
		eStart = CORNER_BOTTOMLEFT;
		eEnd = CORNER_TOPRIGHT;
		eStartDir = eBottomLeft;
		eEndDir = eTopRight;
		break;
	default:
		// Do nothing
		break;
	}

	sStartColour = psColours->aCorner[(int)eStart];
	sEndColour = psColours->aCorner[(int)eEnd];

	TileLine (fX, fY, fWidth, fHeight, eStart, eEnd, eStartDir, eEndDir, & sStartColour, & sEndColour, psRenderData);
}

float GetVolume2D (CelticPersist * psCelticData) {
	float fLength;
	float fCrossSection;
	float fVolume;

	fLength = psCelticData->psRenderData->fLength;
	fCrossSection = M_PI * pow (psCelticData->psRenderData->fThickness, 2.0);
	fVolume = fLength * fCrossSection;

	return fVolume;
}

void SaveSettingsCeltic2D (SettingsPersist * psSettingsData, CelticPersist * psCelticData) {
	SettingsPrintInt (psSettingsData, "Seed", psCelticData->uSeed);
	SettingsPrintInt (psSettingsData, "Width", psCelticData->vnSize.nX);
	SettingsPrintInt (psSettingsData, "Height", psCelticData->vnSize.nY);
	SettingsPrintFloat (psSettingsData, "TileSizeX", psCelticData->vTileSize.fX);
	SettingsPrintFloat (psSettingsData, "TileSizeY", psCelticData->vTileSize.fY);
	SettingsPrintFloat (psSettingsData, "Weirdness", psCelticData->fWeirdness);
	SettingsPrintInt (psSettingsData, "Orientation", psCelticData->eOrientation);
	SettingsPrintBool (psSettingsData, "Symmetrical", psCelticData->boSymmetrify);

	SettingsStartTag (psSettingsData, "render");
	SaveSettingsRender (psSettingsData, psCelticData->psRenderData);
	SettingsEndTag (psSettingsData, "render");
}

void LoadSettingsStartCeltic2D (SettingsPersist * psSettingsData, CelticPersist * psCelticData) {
	SettingsLoadParser * psLoadParser = NULL;

	psLoadParser = g_new0 (SettingsLoadParser, 1);

	psLoadParser->LoadProperty = CelticLoadProperty;
	psLoadParser->LoadSectionStart = CelticLoadSectionStart;
	psLoadParser->LoadSectionEnd = CelticLoadSectionEnd;
	psLoadParser->psData = psCelticData;
	AddParser (psLoadParser, psSettingsData);
}

void LoadSettingsEndCeltic2D (SettingsPersist * psSettingsData, CelticPersist * psCelticData) {
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

bool ExportModel2D (char const * szFilename, bool boBinary, CelticPersist * psCelticData) {
	bool boSuccess;

	boSuccess = OutputStoredBeziers (szFilename, psCelticData->psRenderData->psBezData, boBinary);

	return boSuccess;
}

