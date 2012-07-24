/*******************************************************************
 * Celtic.c
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
#include "celtic_private.h"

#define _USE_MATH_DEFINES
#include <math.h>

/* Defines */
#define MAX_SIZE (1280)
#define WEAVE_HEIGHT (0.3f)
#define CONTROL_SCALE (0.4f)
#define BEZIERS_PER_TILE (8)
#define SelectRandom(ARRAY, NUM) (ARRAY)[(rand() % (NUM))];
#define COLOUR_PLAIN_RED (0.7f)
#define COLOUR_PLAIN_GREEN (0.7f)
#define COLOUR_PLAIN_BLUE (0.7f)
#define COST_PER_UNIT_CUBED (10.0f / (10.0f * 10.0f * 10.0f))

/* Enums */

/* Structures */

/* Function prototypes */
void RenderLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
void RenderLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
void RenderLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);

/* Function defininitions */
void DeleteCelticPersist (CelticPersist * psCelticData) {
	if (psCelticData) {
		if (psCelticData->DeleteCelticPersist) {
			(*psCelticData->DeleteCelticPersist) (psCelticData);
		}
		else {
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
}

RenderPersist * NewRenderPersist (float fXOffset, float fYOffset, float fZOffset) {
	RenderPersist * psRenderData;

	psRenderData = (RenderPersist *)calloc (1, sizeof (RenderPersist));
	psRenderData->fThickness = 0.15f;
	psRenderData->vLineInset.fX = 0.5f;
	psRenderData->vLineInset.fY = 0.5f;
	psRenderData->vLineInset.fZ = 0.5f;
	psRenderData->fWeaveHeight = WEAVE_HEIGHT;
	psRenderData->fControlScale = CONTROL_SCALE;
	psRenderData->vOffset.fX = fXOffset;
	psRenderData->vOffset.fY = fYOffset;
	psRenderData->vOffset.fZ = fZOffset;
	psRenderData->psBezData = NULL;
	psRenderData->psBezierStart = NULL;
	psRenderData->psBezierCurrent = NULL;
	psRenderData->nBezierNum = 0;
	psRenderData->uColourSeed = 0u;
	psRenderData->asColour = NULL;
	psRenderData->uAccuracyLongitudinal = 24u;
	psRenderData->uAccuracyRadial = 10u;
	psRenderData->fLength = 0.0f;

	return psRenderData;
}

void DeleteRenderPersist (RenderPersist * psRenderData) {
	/* Remove any beziers if there are any */
	if (psRenderData->nBezierNum > 0) {
		DeleteBeziers (psRenderData->psBezierStart, psRenderData->nBezierNum, psRenderData->psBezData);
		psRenderData->psBezierStart = NULL;
		psRenderData->nBezierNum = 0;
	}

	if (psRenderData->asColour) {
		free (psRenderData->asColour);
		psRenderData->asColour = NULL;
	}

	if (psRenderData) {
		free (psRenderData);
		psRenderData = NULL;
	}
}

void CopyCelticPersistParams (CelticPersist * psFrom, CelticPersist * psCelticData) {
	psCelticData->uSeed = psFrom->uSeed;
	//psCelticData->nLoops = psFrom->nLoops;
	//psCelticData->aeCorner = psFrom->aeCorner;
	//psCelticData->aeCentre = psFrom->aeCentre;
	psCelticData->vnSize = psFrom->vnSize;
	psCelticData->vTileSize = psFrom->vTileSize;
	psCelticData->fWeirdness = psFrom->fWeirdness;
	psCelticData->eOrientation = psFrom->eOrientation;
	psCelticData->boSymmetrify = psFrom->boSymmetrify;
	psCelticData->boDebug = psFrom->boDebug;

	CopyRenderPersistParams (psFrom->psRenderData, psCelticData->psRenderData);
}

void CopyRenderPersistParams (RenderPersist * psFrom, RenderPersist * psRenderData) {
	psRenderData->fThickness = psFrom->fThickness;
	psRenderData->vLineInset = psFrom->vLineInset;
	psRenderData->fWeaveHeight = psFrom->fWeaveHeight;
	psRenderData->fControlScale = psFrom->fControlScale;
	psRenderData->vOffset = psFrom->vOffset;
	//psRenderData->psBezierStart = psFrom->psBezierStart;
	//psRenderData->psBezierCurrent = psFrom->psBezierCurrent;
	//psRenderData->nBezierNum = psFrom->nBezierNum;
	psRenderData->uColourSeed = psFrom->uColourSeed;
	//psRenderData->asColour = psFrom->asColour;
	//psRenderData->fLength = psFrom->fLength;
}

/* Main function */
void GenerateKnot (CelticPersist * psCelticData) {
	if (psCelticData->GenerateKnot) {
		(*psCelticData->GenerateKnot) (psCelticData);
	}
}

void SetCelticBezData (BezPersist * psBezData, CelticPersist * psCelticData) {
	psCelticData->psRenderData->psBezData = psBezData;

	if (psBezData == NULL) {
		psCelticData->psRenderData->psBezierStart = NULL;
		psCelticData->psRenderData->psBezierCurrent = NULL;
		psCelticData->psRenderData->nBezierNum = 0;
	}
}

/* Render the knots to the canvas */
void RenderKnots (CelticPersist * psCelticData) {
	if (psCelticData->RenderKnots) {
		(*psCelticData->RenderKnots) (psCelticData);
	}
}

bool ExportModel (char const * szFilename, bool boBinary, CelticPersist * psCelticData) {
	bool boSuccess = FALSE;

	if (psCelticData->ExportModel) {
		boSuccess = (*psCelticData->ExportModel) (szFilename, boBinary, psCelticData);
	}

	return boSuccess;
}

bool SetTileX (float fTileX, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->vTileSize.fX != fTileX);
	psCelticData->vTileSize.fX = fTileX;
	psCelticData->psRenderData->vOffset.fX = -(((float)psCelticData->vnSize.nX) * fTileX / 2.0f);

	return boChanged;
}

bool SetTileY (float fTileY, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->vTileSize.fY != fTileY);
	psCelticData->vTileSize.fY = fTileY;
	psCelticData->psRenderData->vOffset.fY = -(((float)psCelticData->vnSize.nY) * fTileY / 2.0f);

	return boChanged;
}

bool SetTileZ (float fTileZ, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->vTileSize.fZ != fTileZ);
	psCelticData->vTileSize.fZ = fTileZ;
	psCelticData->psRenderData->vOffset.fZ = -(((float)psCelticData->vnSize.nZ) * fTileZ / 2.0f);

	return boChanged;
}

float GetTileX (CelticPersist * psCelticData) {
	return psCelticData->vTileSize.fX;
}

float GetTileY (CelticPersist * psCelticData) {
	return psCelticData->vTileSize.fY;
}

float GetTileZ (CelticPersist * psCelticData) {
	return psCelticData->vTileSize.fZ;
}

bool SetThickness (float fThickness, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->psRenderData->fThickness != fThickness);
	psCelticData->psRenderData->fThickness = fThickness;

	return boChanged;
}

float GetThickness (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->fThickness;
}

bool SetWeaveHeight (float fWeaveHeight, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->psRenderData->fWeaveHeight != fWeaveHeight);
	psCelticData->psRenderData->fWeaveHeight = fWeaveHeight;

	return boChanged;
}

float GetWeaveHeight (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->fWeaveHeight;
}

bool SetControlScale (float fControlScale, CelticPersist * psCelticData) {
	bool boChanged;
	
	boChanged = (psCelticData->psRenderData->fControlScale != fControlScale);
	psCelticData->psRenderData->fControlScale = fControlScale;
	
	return boChanged;
}

float GetControlScale (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->fControlScale;
}

bool SetInsetX (float fInsetX, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->psRenderData->vLineInset.fX != fInsetX);
	psCelticData->psRenderData->vLineInset.fX = fInsetX;

	return boChanged;
}

bool SetInsetY (float fInsetY, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->psRenderData->vLineInset.fY != fInsetY);
	psCelticData->psRenderData->vLineInset.fY = fInsetY;

	return boChanged;
}

bool SetInsetZ (float fInsetZ, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->psRenderData->vLineInset.fZ != fInsetZ);
	psCelticData->psRenderData->vLineInset.fZ = fInsetZ;

	return boChanged;
}

float GetInsetX (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->vLineInset.fX;
}

float GetInsetY (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->vLineInset.fY;
}

float GetInsetZ (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->vLineInset.fZ;
}

bool SetSymmetrify (bool boSymmetrify, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->boSymmetrify != boSymmetrify);
	psCelticData->boSymmetrify = boSymmetrify;

	return boChanged;
}

bool GetSymmetrify (CelticPersist * psCelticData) {
	return psCelticData->boSymmetrify;
}

bool SetWeirdness (float fWeirdness, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->fWeirdness != fWeirdness);
	psCelticData->fWeirdness = fWeirdness;

	return boChanged;
}

float GetWeirdness (CelticPersist * psCelticData) {
	return psCelticData->fWeirdness;
}

bool SetWidth (int nWidth, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->vnSize.nX != nWidth);
	psCelticData->vnSize.nX = nWidth;
	psCelticData->psRenderData->vOffset.fX = -(((float)psCelticData->vnSize.nX) * psCelticData->vTileSize.fX / 2.0f);

	return boChanged;
}

bool SetHeight (int nHeight, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->vnSize.nY != nHeight);
	psCelticData->vnSize.nY = nHeight;
	psCelticData->psRenderData->vOffset.fY = -(((float)psCelticData->vnSize.nY) * psCelticData->vTileSize.fY / 2.0f);

	return boChanged;
}

bool SetDepth (int nDepth, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->vnSize.nZ != nDepth);
	psCelticData->vnSize.nZ = nDepth;
	psCelticData->psRenderData->vOffset.fZ = -(((float)psCelticData->vnSize.nZ) * psCelticData->vTileSize.fZ / 2.0f);

	return boChanged;
}

int GetWidth (CelticPersist * psCelticData) {
	return psCelticData->vnSize.nX;
}

int GetHeight (CelticPersist * psCelticData) {
	return psCelticData->vnSize.nY;
}

int GetDepth (CelticPersist * psCelticData) {
	return psCelticData->vnSize.nZ;
}

bool SetOrientation (TILE eOrientation, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->eOrientation != eOrientation);
	psCelticData->eOrientation = eOrientation;

	return boChanged;
}

TILE GetOrientation (CelticPersist * psCelticData) {
	return psCelticData->eOrientation;
}

bool SetSeed (unsigned int uSeed, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->uSeed != uSeed);
	psCelticData->uSeed = uSeed;

	return boChanged;
}

unsigned int GetSeed (CelticPersist * psCelticData) {
	return psCelticData->uSeed;
}

bool SetColourSeed (unsigned int uSeed, CelticPersist * psCelticData) {
	bool boChanged;

	boChanged = (psCelticData->psRenderData->uColourSeed != uSeed);
	psCelticData->psRenderData->uColourSeed = uSeed;

	return boChanged;
}

unsigned int GetColourSeed (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->uColourSeed;
}

bool SetAccuracyLongitudinal (unsigned int uAccuracy, CelticPersist * psCelticData) {
	bool boChanged = FALSE;

	if (uAccuracy >= 1) {
		boChanged = (psCelticData->psRenderData->uAccuracyLongitudinal != uAccuracy);
		psCelticData->psRenderData->uAccuracyLongitudinal = uAccuracy;

		SetAccuracy (psCelticData->psRenderData->uAccuracyLongitudinal, psCelticData->psRenderData->uAccuracyRadial, psCelticData->psRenderData->psBezData);
	}

	return boChanged;
}

unsigned int GetAccuracyLongitudinal (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->uAccuracyLongitudinal;
}

bool SetAccuracyRadial (unsigned int uAccuracy, CelticPersist * psCelticData) {
	bool boChanged = FALSE;

	if (uAccuracy >= 3) {
		boChanged = (psCelticData->psRenderData->uAccuracyRadial != uAccuracy);
		psCelticData->psRenderData->uAccuracyRadial = uAccuracy;

		SetAccuracy (psCelticData->psRenderData->uAccuracyLongitudinal, psCelticData->psRenderData->uAccuracyRadial, psCelticData->psRenderData->psBezData);
	}

	return boChanged;
}

unsigned int GetAccuracyRadial (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->uAccuracyRadial;
}

int GetLoops (CelticPersist * psCelticData) {
	return psCelticData->nLoops;
}

float GetLength (CelticPersist * psCelticData) {
	return psCelticData->psRenderData->fLength;
}

float GetVolume (CelticPersist * psCelticData) {
	float fVolume = 0.0f;

	if (psCelticData->GetVolume) {
		fVolume = (*psCelticData->GetVolume) (psCelticData);
	}

	return fVolume;
}

void SaveSettingsCeltic (SettingsPersist * psSettingsData, CelticPersist * psCelticData) {
	if (psCelticData->SaveSettingsCeltic) {
		(*psCelticData->SaveSettingsCeltic) (psSettingsData, psCelticData);
	}
}

void LoadSettingsStartCeltic (SettingsPersist * psSettingsData, CelticPersist * psCelticData) {
	if (psCelticData->LoadSettingsStartCeltic) {
		(*psCelticData->LoadSettingsStartCeltic) (psSettingsData, psCelticData);
	}
}

void LoadSettingsEndCeltic (SettingsPersist * psSettingsData, CelticPersist * psCelticData) {
	if (psCelticData->LoadSettingsEndCeltic) {
		(*psCelticData->LoadSettingsEndCeltic) (psSettingsData, psCelticData);
	}
}

void SaveSettingsRender (SettingsPersist * psSettingsData, RenderPersist * psRenderData) {
	SettingsPrintInt (psSettingsData, "ColourSeed", psRenderData->uColourSeed);
	SettingsPrintFloat (psSettingsData, "LineInsetX", psRenderData->vLineInset.fX);
	SettingsPrintFloat (psSettingsData, "LineInsetY", psRenderData->vLineInset.fY);
	SettingsPrintFloat (psSettingsData, "LineInsetZ", psRenderData->vLineInset.fZ);
	SettingsPrintFloat (psSettingsData, "Thickness", psRenderData->fThickness);
	SettingsPrintFloat (psSettingsData, "WeaveHeight", psRenderData->fWeaveHeight);
	SettingsPrintInt (psSettingsData, "AccuracyLongitudinal", psRenderData->uAccuracyLongitudinal);
	SettingsPrintInt (psSettingsData, "AccuracyRadial", psRenderData->uAccuracyRadial);
}

void LoadSettingsStartRender (SettingsPersist * psSettingsData, RenderPersist * psRenderData) {
	SettingsLoadParser * psLoadParser = NULL;

	psLoadParser = g_new0 (SettingsLoadParser, 1);

	psLoadParser->LoadProperty = RenderLoadProperty;
	psLoadParser->LoadSectionStart = RenderLoadSectionStart;
	psLoadParser->LoadSectionEnd = RenderLoadSectionEnd;
	psLoadParser->psData = psRenderData;
	AddParser (psLoadParser, psSettingsData);
}

void LoadSettingsEndRender (SettingsPersist * psSettingsData, RenderPersist * psRenderData) {
	SettingsLoadParser * psLoadParser = NULL;

	psLoadParser = GetParser (psSettingsData);
	g_free (psLoadParser);
	RemoveParser (psSettingsData);
}

void RenderLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
	RenderPersist * psRenderData = (RenderPersist *)psData;

	switch (eType) {
	case SETTINGTYPE_FLOAT:
		if (stricmp (szName, "LineInsetX") == 0) {
			psRenderData->vLineInset.fX = *((float*)(psValue));
		}
		else if (stricmp (szName, "LineInsetY") == 0) {
			psRenderData->vLineInset.fY = *((float*)(psValue));
		}
		else if (stricmp (szName, "LineInsetZ") == 0) {
			psRenderData->vLineInset.fZ = *((float*)(psValue));
		}
		else if (stricmp (szName, "Thickness") == 0) {
			psRenderData->fThickness = *((float*)(psValue));
		}
		else if (stricmp (szName, "WeaveHeight") == 0) {
			psRenderData->fWeaveHeight = *((float*)(psValue));
		}
		break;
	case SETTINGTYPE_INT:
		if (stricmp (szName, "ColourSeed") == 0) {
			psRenderData->uColourSeed = *((unsigned int*)(psValue));
		}
		else if (stricmp (szName, "AccuracyLongitudinal") == 0) {
			psRenderData->uAccuracyLongitudinal = *((unsigned int*)(psValue));
		}
		else if (stricmp (szName, "AccuracyRadial") == 0) {
			psRenderData->uAccuracyRadial = *((unsigned int*)(psValue));
		}
		break;
	default:
		printf ("Unknown celtic property %s\n", szName);
		break;
	}
}

void RenderLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {

	//RenderPersist * psRenderData = (RenderPersist *)psData;
	// Any subsections go in here
}

void RenderLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
	RenderPersist * psRenderData = (RenderPersist *)psData;

	if (stricmp (szName, "render") == 0) {
		// Move out of the render section
		LoadSettingsEndRender (psSettingsData, psRenderData);
	}
}


