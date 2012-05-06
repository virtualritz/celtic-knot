/*******************************************************************
 * Celtic_private.h
 * Private data structures for generating celtic knot patterns
 *
 * David Llewellyn-Jones
 * http://www.flypig.co.uk
 *
 * October 2011
 *******************************************************************
*/

#ifndef CELTIC_PRIVATE_H
#define CELTIC_PRIVATE_H

/* Includes */

#include "vecint.h"

/* Defines */

#define COLOUR_PLAIN_RED (0.7f)
#define COLOUR_PLAIN_GREEN (0.7f)
#define COLOUR_PLAIN_BLUE (0.7f)

#define CUBE(NAME, DIMENSION, TYPE) typedef struct _Cube##DIMENSION##NAME { \
	TYPE aCorner[(2 << (DIMENSION))]; \
} Cube##DIMENSION##NAME;

/* Enums */

/* Structures */

typedef struct _ColFloats {
	float fRed;
	float fGreen;
	float fBlue;
} ColFloats;

CUBE (Colour, 3, ColFloats)
CUBE (Complete, 3, bool)

CUBE (Colour, 2, ColFloats)
CUBE (Complete, 2, bool)

struct _RenderPersist {
	BezPersist * psBezData;
	float fThickness;
	Vector3 vLineInset;
	float fWeaveHeight;
	float fControlScale;
	Vector3 vOffset;
	Bezier * psBezierStart;
	Bezier * psBezierCurrent;
	int nBezierNum;
	unsigned int uColourSeed;
	union {
		Cube2Colour * asColour2D;
		Cube3Colour * asColour3D;
		void * asColour;
	};
	float fLength;
};

struct _CelticPersist {
	/* Variables */
	unsigned int uSeed;
	int nLoops;
	TILE * aeCorner;
	TILE * aeCentre;
	VecInt3 vnSize;
	Vector3 vTileSize;
	float fWeirdness;
	TILE eOrientation;
	bool boSymmetrify;
	bool boDebug;
	RenderPersist * psRenderData;

	/* Virtual functions */
	void (*DeleteCelticPersist) (CelticPersist * psCelticData);

	void (*GenerateKnot) (CelticPersist * psCelticData);
	void (*RenderKnots) (CelticPersist * psCelticData);

	float (*GetVolume) (CelticPersist * psCelticData);

	void (*SaveSettingsCeltic) (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
	void (*LoadSettingsStartCeltic) (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
	void (*LoadSettingsEndCeltic) (SettingsPersist * psSettingsData, CelticPersist * psCelticData);
	
	bool (*ExportModel) (char const * szFilename, bool boBinary, CelticPersist * psCelticData);
};

/* Function prototypes */

RenderPersist * NewRenderPersist (float fXOffset, float fYOffset, float fZOffset);
void DeleteRenderPersist (RenderPersist * psRenderData);
void SaveSettingsRender (SettingsPersist * psSettingsData, RenderPersist * psRenderData);
void LoadSettingsStartRender (SettingsPersist * psSettingsData, RenderPersist * psRenderData);
void LoadSettingsEndRender (SettingsPersist * psSettingsData, RenderPersist * psRenderData);

#endif /* CELTIC_PRIVATE_H */

