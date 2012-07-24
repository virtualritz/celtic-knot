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

#ifndef CELTIC_H
#define CELTIC_H

/* Includes */

#include "bez.h"
#include "utils.h"
#include "settings.h"

/* Defines */

/* Enums */

typedef enum {
	TILE_INVALID = -1,

	TILE_CROSS,
	TILE_HORIZONTAL,
	TILE_VERTICAL,
	TILE_LONGITUDINAL,

	TILE_NUM
} TILE;

typedef enum {
	CORNER_INVALID = -1,

	CORNER_TOPLEFT,
	CORNER_TOPRIGHT,
	CORNER_BOTTOMLEFT,
	CORNER_BOTTOMRIGHT,

	CORNER_NUM
} CORNER;

/* Structures */
typedef struct _RenderPersist RenderPersist;

typedef struct _CelticPersist CelticPersist;

/* Function prototypes */
void DeleteCelticPersist (CelticPersist * psCelticData);
void CopyCelticPersistParams (CelticPersist * psFrom, CelticPersist * psCelticData);
void CopyRenderPersistParams (RenderPersist * psFrom, RenderPersist * psRenderData);
void GenerateKnot (CelticPersist * psCelticData);
void RenderKnots (CelticPersist * psCelticData);

/* Management propertes */
void SetCelticBezData (BezPersist * psBezData, CelticPersist * psCelticData);

/* Static properties */
bool SetSeed (unsigned int uSeed, CelticPersist * psCelticData);
unsigned int GetSeed (CelticPersist * psCelticData);
bool SetColourSeed (unsigned int uSeed, CelticPersist * psCelticData);
unsigned int GetColourSeed (CelticPersist * psCelticData);
bool SetSymmetrify (bool boSymmetrify, CelticPersist * psCelticData);
bool GetSymmetrify (CelticPersist * psCelticData);
bool SetWeirdness (float fWeirdness, CelticPersist * psCelticData);
float GetWeirdness (CelticPersist * psCelticData);
bool SetWidth (int nWidth, CelticPersist * psCelticData);
bool SetHeight (int nHeight, CelticPersist * psCelticData);
bool SetDepth (int nDepth, CelticPersist * psCelticData);
int GetWidth (CelticPersist * psCelticData);
int GetHeight (CelticPersist * psCelticData);
int GetDepth (CelticPersist * psCelticData);
bool SetOrientation (TILE eOrientation, CelticPersist * psCelticData);
TILE GetOrientation (CelticPersist * psCelticData);
bool SetAccuracyLongitudinal (unsigned int uAccuracy, CelticPersist * psCelticData);
unsigned int GetAccuracyLongitudinal (CelticPersist * psCelticData);
bool SetAccuracyRadial (unsigned int uAccuracy, CelticPersist * psCelticData);
unsigned int GetAccuracyRadial (CelticPersist * psCelticData);

/* Dynamic properties */
bool SetTileX (float fTileX, CelticPersist * psCelticData);
bool SetTileY (float fTileY, CelticPersist * psCelticData);
bool SetTileZ (float fTileZ, CelticPersist * psCelticData);
float GetTileX (CelticPersist * psCelticData);
float GetTileY (CelticPersist * psCelticData);
float GetTileZ (CelticPersist * psCelticData);
bool SetThickness (float fThickness, CelticPersist * psCelticData);
float GetThickness (CelticPersist * psCelticData);
bool SetWeaveHeight (float fWeaveHeight, CelticPersist * psCelticData);
float GetWeaveHeight (CelticPersist * psCelticData);
bool SetControlScale (float fControlScale, CelticPersist * psCelticData);
float GetControlScale (CelticPersist * psCelticData);
bool SetInsetX (float fInsetX, CelticPersist * psCelticData);
bool SetInsetY (float fInsetY, CelticPersist * psCelticData);
bool SetInsetZ (float fInsetZ, CelticPersist * psCelticData);
float GetInsetX (CelticPersist * psCelticData);
float GetInsetY (CelticPersist * psCelticData);
float GetInsetZ (CelticPersist * psCelticData);

/* Characteristics */
int GetLoops (CelticPersist * psCelticData);
float GetLength (CelticPersist * psCelticData);
float GetVolume (CelticPersist * psCelticData);

/* Settings loading/saving */
void SaveSettingsCeltic (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
void LoadSettingsStartCeltic (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
void LoadSettingsEndCeltic (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
bool ExportModel (char const * szFilename, bool boBinary, CelticPersist * psCelticData);

#endif /* CELTIC_H */

