///////////////////////////////////////////////////////////////////
// Trustella Network
// Results log file animation viewer
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Autumn 2006
///////////////////////////////////////////////////////////////////

#ifndef VIS_H
#define VIS_H

///////////////////////////////////////////////////////////////////
// Includes

typedef struct _VisPersist VisPersist;

#include "utils.h"
#include "settings.h"
#include "celtic.h"

///////////////////////////////////////////////////////////////////
// Defines

//#define LEFT_BUTTON GLUT_LEFT_BUTTON
//#define RIGHT_BUTTON GLUT_RIGHT_BUTTON
//#define BUTTON_DOWN GLUT_DOWN
//#define BUTTON_UP GLUT_UP
#define LEFT_BUTTON 1
#define RIGHT_BUTTON 3
#define BUTTON_DOWN GDK_BUTTON_PRESS
#define BUTTON_UP GDK_BUTTON_RELEASE

#define SCREENWIDTH					(800)
#define SCREENHEIGHT				(600)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

VisPersist * NewVisPersist ();
void DeleteVisPersist (VisPersist * psVisData);
void Redraw (VisPersist * psVisData);
void Mouse (int button, int state, int x, int y, VisPersist * psVisData);
void Key (unsigned char key, int x, int y, unsigned int uKeyModifiers, VisPersist * psVisData);
void KeyUp (unsigned char key, int x, int y, unsigned int uKeyModifiers, VisPersist * psVisData);
void Motion (int nX, int nY, VisPersist * psVisData);
void Reshape (int w, int h, VisPersist * psVisData);
void Idle (VisPersist * psVisData);
void Init (VisPersist * psVisData);
void Realise (VisPersist * psVisData);
void Unrealise (VisPersist * psVisData);
void ToggleFullScreen (VisPersist * psVisData);
void ToggleClearWhite (VisPersist * psVisData);
void ToggleSpin (VisPersist * psVisData);
void GetDisplayProperties (float * pfViewRadius, float * pfTileX, float * pfTileY, float * pfTileZ, float * pfThickness, float * pfWeave, float * pfInsetX, float * pfInsetY, float * pfInsetZ, float * pfCurve, VisPersist * psVisData);
void SetDisplayProperties (float fViewRadius, float fTileX, float fTileY, float fTileZ, float fThickness, float fWeave, float fInsetX, float fInsetY, float fInsetZ, float fCurve, VisPersist * psVisData);
void SetClearWhite (bool boClearWhite, VisPersist * psVisData);
bool GetClearWhite (VisPersist * psVisData);
bool GetFullScreen (VisPersist * psVisData);
gboolean GetMoving (VisPersist * psVisData);
int GetDimensions (VisPersist * psVisData);
void SetDimensions (int nDimensions, VisPersist * psVisData);
void RenderBitmapString (float fX, float fY, void * pFont, char const * szString);
void SaveSettingsVis (SettingsPersist * psSettingsData, VisPersist * psVisData);
void LoadSettingsStartVis (SettingsPersist * psSettingsData, VisPersist * psVisData);
void LoadSettingsEndVis (SettingsPersist * psSettingsData, VisPersist * psVisData);
CelticPersist * GetCelticData (VisPersist * psVisData);
unsigned int GetRandomSeed ();
bool ExportBitmap (char const * szFilename, char const * szType, int nHeight, int nWidth, VisPersist * psVisData);
int GetScreenHeight (VisPersist * psVisData);
int GetScreenWidth (VisPersist * psVisData);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* VIS_H */


