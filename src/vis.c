///////////////////////////////////////////////////////////////////
// Visualiser
// Display the actual 3D objects
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Winter 2010/2011
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <stdlib.h>
#include <glib.h>

#include "vis.h"
#include "bez.h"
#include "shader.h"
#include "celtic.h"
#include "celtic3d.h"
#include "celtic2d.h"

///////////////////////////////////////////////////////////////////
// Defines

#define true 								(1)
#define false 							(0)

#define MOUSE_ROTATE_SCALE	(100.0f)
#define RADIUSSTEP					(0.3f)
#define PSISTEP							(3.14159265f / 150.0f)

#define MAXKEYS							(256)
#define SELBUFSIZE					(512)

#define VIEW_RADIUS					(15.0f)

#define TEXT_FONT						(GLUT_BITMAP_HELVETICA_10)
#define TEXT_XOFF						(12)
#define TEXT_YOFF						(12)
#define TEXT_COLOUR					0.5, 1.0, 0.5

#define MOMENTUM_MIN				(0.0005f)
#define MOMENTUM_RESISTANCE	(0.98f)

#define VERTEX_BUFFERS			(1)
#define INDEX_BUFFERS				(50)

#define TUBE_PIECES 				(10)
#define TUBE_SEGMENTS 			(24)

#define NUMBER_STRING_MAX (16)
#define LINEHEIGHT (12)

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef enum {
	SELECTOBJECT_INVALID = -1,

	SELECTOBJECT_NUM
} SELECTOBJECT;

typedef enum {
	DRAG_INVALID = -1,

	DRAG_NONE,
	DRAG_VIEW,

	DRAG_NUM
} DRAG;

typedef struct _SelectInfo {
	// Values in
	int nXPos;
	int nYPos;

	// Values out
	SELECTOBJECT eObjectType;
} SelectInfo;

struct _VisPersist {
	double fCurrentTime;
	double fPrevTime;
	double fSpinTime;
	float fViewRadius;
	float fX;
	float fY;
	float fZ;
	float fXn;
	float fYn;
	float fZn;
	bool boSpin;
	int nXMouse;
	int nYMouse;
	DRAG eDrag;
	SELECTOBJECT eSelectObject;
	int nScreenWidth;
	int nScreenHeight;
	int nPrevScreenWidth;
	int nPrevScreenHeight;
	bool boFullScreen;
	bool boClearWhite;
	bool aboKeyDown[MAXKEYS];
	bool boDebug;
	float fMomentum;
	float fXMomentum;
	float fYMomentum;
	float fZMomentum;
	int nDimensions;
	GLuint uStartList;
	GLuint auVertexBuffer[VERTEX_BUFFERS];
	GLuint auIndexBuffer[INDEX_BUFFERS];
	BezPersist * psBezData;
	CelticPersist * psCelticData;
	ShaderPersist * psShaderData;
};

typedef struct _KnotVertex {
	float fX;
	float fY;
	float fZ;
	float fNX;
	float fNY;
	float fNZ;
	//float fPadding[2];
} KnotVertex;

///////////////////////////////////////////////////////////////////
// Global variables

static GLfloat gafDiffuse[] = { 0.5, 0.5, 1.0, 1.0 };
static GLfloat gafSpecular[] = { 0.7, 0.7, 0.7, 1.0 };
static GLfloat gafShininess[] = { 10.0 };

///////////////////////////////////////////////////////////////////
// Function prototypes

void Render (VisPersist * psVisData);
void RenderTextInSpace (char const * szText, GLdouble fX, GLdouble fY, GLdouble fZ);
void DrawTextOverlay (VisPersist * psVisData);
void ChangeView (float fTheta, float fPhi, float fPsi, float fRadius, VisPersist * psVisData);
void KeyIdle (VisPersist * psVisData);
void ResetAnimation (VisPersist * psVisData);
void Spin (VisPersist * psVisData);
void MomentumSpin (VisPersist * psVisData);
void InitialiseDisplayLists (VisPersist * psVisData);
void DeleteDisplayLists (VisPersist * psVisData);
void MouseDown (int nXPos, int nYPos, VisPersist * psVisData);
void SelectVisObject (SelectInfo * psSelectData, VisPersist * psVisData);
void DrawObject (GLdouble fXCentre, GLdouble fYCentre, GLdouble fZCentre, VisPersist * psVisData);
void VisLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
void VisLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
void VisLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);
void InitShaders (VisPersist * psVisData);
void FreePixelBuffer (guchar * pixels, gpointer data);

///////////////////////////////////////////////////////////////////
// Function definitions

VisPersist * NewVisPersist (void) {
	VisPersist * psVisData;

	psVisData = g_new0 (VisPersist, 1);

	psVisData->fViewRadius = VIEW_RADIUS;
	psVisData->fX = 0.0f;
	psVisData->fY = 0.0f;
	psVisData->fZ = 1.0f;
	psVisData->fXn = 0.0f;
	psVisData->fYn = 1.0f;
	psVisData->fZn = 0.0f;
	psVisData->boSpin = FALSE;
	psVisData->nXMouse = 0;
	psVisData->nYMouse = 0;
	psVisData->eDrag = DRAG_NONE;
	psVisData->eSelectObject = SELECTOBJECT_INVALID;
	psVisData->nScreenWidth = SCREENWIDTH;
	psVisData->nScreenHeight = SCREENHEIGHT;
	psVisData->nPrevScreenWidth = SCREENWIDTH;
	psVisData->nPrevScreenHeight = SCREENHEIGHT;
	psVisData->boFullScreen = false;
	psVisData->boClearWhite = false;
	psVisData->boDebug = false;

	psVisData->fMomentum = 0.0f;
	psVisData->fXMomentum = 0.0f;
	psVisData->fYMomentum = 0.0f;
	psVisData->fZMomentum = 0.0f;

	psVisData->uStartList = 0;
	
	psVisData->psBezData = NULL;
	
	psVisData->nDimensions = 2;
	//psVisData->psCelticData = NewCelticPersist3D (3, 4, 3, 3.0f, 3.0f, 3.0f, psVisData->psBezData);
	psVisData->psCelticData = NewCelticPersist2D (5, 6, 3.0f, 3.0f, psVisData->psBezData);
	SetDepth (5, psVisData->psCelticData);
	SetTileZ (3.0f, psVisData->psCelticData);
	psVisData->psShaderData = NULL;

	return psVisData;
}

void DeleteVisPersist (VisPersist * psVisData) {
	if (psVisData->psCelticData) {
		DeleteCelticPersist (psVisData->psCelticData);
		psVisData->psCelticData = NULL;
	}
	
	if (psVisData->psBezData) {
		DeleteBezPersist (psVisData->psBezData);
		psVisData->psBezData = NULL;
	}

	if (psVisData->psShaderData) {
		DeleteShaderPersist (psVisData->psShaderData);
		psVisData->psShaderData = NULL;
	}

	g_free (psVisData);
}

void Realise (VisPersist * psVisData) {
	GLfloat light_ambient[] = { 0.3, 0.3, 0.3, 1.0, 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0, 0.5, 0.3, 0.0, 1.0 };
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0, 0.3, 0.5, 1.0, 0.0 };
	GLfloat light_position[] = { 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0 };

	gluLookAt (0.0, VIEW_RADIUS, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0);

	//glLightfv (GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv (GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv (GL_LIGHT0, GL_SPECULAR, light_specular);
	glLightfv (GL_LIGHT0, GL_POSITION, light_position);
	glLightModelfv (GL_LIGHT_MODEL_AMBIENT, light_ambient);
	glLightModeli (GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	//glLightfv (GL_LIGHT1, GL_AMBIENT, light_ambient + 4);
	//glLightfv (GL_LIGHT1, GL_DIFFUSE, light_diffuse + 4);
	//glLightfv (GL_LIGHT1, GL_SPECULAR, light_specular + 4);
	//glLightfv (GL_LIGHT1, GL_POSITION, light_position + 4);

	glFrontFace (GL_CCW);
	glEnable (GL_LIGHTING);
	glEnable (GL_LIGHT0);
	//glEnable(GL_LIGHT1);
	glEnable (GL_CULL_FACE);
	glCullFace (GL_BACK);
	glEnable (GL_DEPTH_TEST);
	//glEnable(GL_TEXTURE_2D);
	glHint (GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
	glClearColor (0.0f, 0.0f, 0.0f, 0.0f);

	InitialiseDisplayLists (psVisData);

	InitShaders (psVisData);

	psVisData->psBezData = NewBezPersist (TUBE_PIECES, TUBE_SEGMENTS);
	SetCelticBezData (psVisData->psBezData, psVisData->psCelticData);

	//psVisData->psCelticData = NewCelticPersist (7, 7, 10, 10);
	GenerateKnot (psVisData->psCelticData);

	RenderKnots (psVisData->psCelticData);
}

void Unrealise (VisPersist * psVisData) {
	// Delete the OpenGL display lists
	DeleteDisplayLists (psVisData);

	DeleteBezPersist (psVisData->psBezData);
	psVisData->psBezData = NULL;
	SetCelticBezData (psVisData->psBezData, psVisData->psCelticData);
}

void Init (VisPersist * psVisData) {
	int nCount;
	struct timeb sTime;

	for (nCount = 0; nCount < MAXKEYS; nCount++) {
		psVisData->aboKeyDown[nCount] = false;
	}

	ftime (& sTime);
	psVisData->fCurrentTime = (double)(sTime.time) + (double)(sTime.millitm) / 1000.0;
	if (psVisData->fCurrentTime < 0) psVisData->fCurrentTime = 0.0;
	psVisData->fPrevTime = psVisData->fCurrentTime;
	psVisData->fSpinTime = psVisData->fCurrentTime;
}

void InitShaders (VisPersist * psVisData) {

	if (psVisData->psShaderData == 0) {
		psVisData->psShaderData = NewShaderPersist ();
	}

	LoadVertexShader (KNOTDIR "/vertex.vs", psVisData->psShaderData);
	LoadFragmentShader (KNOTDIR "/fragment.fs", psVisData->psShaderData);
	ActivateShader (psVisData->psShaderData);
}

void Spin (VisPersist * psVisData) {
	double fTimeChange;

	fTimeChange = psVisData->fCurrentTime - psVisData->fSpinTime;
	if (fTimeChange > 0.005f) {
		if (fTimeChange > 0.05f) {
			fTimeChange = 0.05f;
		}
		ChangeView (0.2f * fTimeChange, 0.005f * sin (0.11f * psVisData->fCurrentTime), 0.0f, 0.0f,
			psVisData);
		psVisData->fSpinTime = psVisData->fCurrentTime;
	}
}

void RenderTextInSpace (char const * szText, GLdouble fX, GLdouble fY, GLdouble fZ) {
	GLdouble afModel[16];
	GLdouble afProjection[16];
	GLint anViewpoert[4];

	glGetDoublev (GL_MODELVIEW_MATRIX, afModel);
	glGetDoublev (GL_PROJECTION_MATRIX, afProjection);
	glGetIntegerv (GL_VIEWPORT, anViewpoert);

	gluProject (fX, fY, fZ, afModel, afProjection, anViewpoert, & fX, & fY, & fZ);

	glDisable (GL_LIGHTING);
	glColor3f (TEXT_COLOUR);
	fX += TEXT_XOFF;
	fY += TEXT_YOFF;

	RenderBitmapString ((float)fX, (float)fY, TEXT_FONT, szText);
	glEnable (GL_LIGHTING);
}

void InitialiseDisplayLists (VisPersist * psVisData) {
	GLUquadricObj * sQobj;

	psVisData->uStartList = glGenLists (1);

	int error = glGetError();
	if (error != GL_NO_ERROR) {
		fprintf (stderr, "Error %s\n", gluErrorString(error));
	}

	sQobj = gluNewQuadric ();
	gluQuadricDrawStyle (sQobj, GLU_FILL);
	gluQuadricNormals (sQobj, GLU_SMOOTH);

	glNewList (psVisData->uStartList, GL_COMPILE);
	gluCylinder (sQobj, 10.0, 10.0, 1.0, 10, 10);
	glEndList ();

	gluDeleteQuadric (sQobj);
}

void DeleteDisplayLists (VisPersist * psVisData) {
	glDeleteLists (psVisData->uStartList, 1);
	psVisData->uStartList = 0;
}

void Render (VisPersist * psVisData) {
	glLoadIdentity ();
	gluLookAt ((psVisData->fViewRadius) * psVisData->fX, (psVisData->fViewRadius) * psVisData->fY, (psVisData->fViewRadius) * psVisData->fZ, 0.0, 0.0, 0.0, psVisData->fXn, psVisData->fYn, psVisData->fZn);

	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gafDiffuse);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, gafSpecular);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, gafShininess);

	DrawObject (0.0, 0.0, 0.0, psVisData);

	DisplayBeziers (psVisData->psBezData);

	glFlush ();
}

void DrawObject (GLdouble fXCentre, GLdouble fYCentre, GLdouble fZCentre, VisPersist * psVisData) {
	glPushMatrix ();
	glTranslatef (fXCentre, fYCentre, fZCentre);

	//glutSolidCube (0.5);
	glPopMatrix ();
}

void SelectVisObject (SelectInfo * psSelectData, VisPersist * psVisData) {
	GLint anViewPort[4];
	GLuint auSelectBuffer[SELBUFSIZE];
	unsigned int uCount;
	unsigned int uOrder;
	int nHits;
	int nBufferPos;
	int nNamesNum;
	SELECTOBJECT eObjectType;

	glSelectBuffer (SELBUFSIZE, auSelectBuffer);
	glRenderMode (GL_SELECT);

	glGetIntegerv (GL_VIEWPORT, anViewPort);

	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	gluPickMatrix (psSelectData->nXPos, anViewPort[3] - psSelectData->nYPos, 1, 1, anViewPort);

	gluPerspective (60, (float)psVisData->nScreenWidth / (float)psVisData->nScreenHeight, 1, 100);
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();

	glInitNames ();

	gluLookAt ((psVisData->fViewRadius) * psVisData->fX, (psVisData->fViewRadius) * psVisData->fY, (psVisData->fViewRadius) * psVisData->fZ, 0.0, 0.0, 0.0, psVisData->fXn, psVisData->fYn, psVisData->fZn);

	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();
	glMatrixMode (GL_MODELVIEW);
	glPopMatrix ();
	glFlush ();

	nHits = glRenderMode (GL_RENDER);
	// Find the selected object
	eObjectType = SELECTOBJECT_INVALID;
	if (nHits != 0) {
		nBufferPos = 0;
		uOrder = -1;
		for (uCount = 0; uCount < nHits; uCount++) {
			nNamesNum = auSelectBuffer[nBufferPos];
			nBufferPos += 3;

			if ((auSelectBuffer[nBufferPos - 1] <= uOrder) && (nNamesNum >= 2)) {
				eObjectType = auSelectBuffer[nBufferPos];
				//nNodeSelected = auSelectBuffer[nBufferPos + 1];
				if (nNamesNum >= 3) {
					//nLinkSelected = auSelectBuffer[nBufferPos + 2];
				}
				uOrder = auSelectBuffer[nBufferPos - 1];
			}

			nBufferPos += nNamesNum;
		}
	}

	psSelectData->eObjectType = eObjectType;
}

void DrawTextOverlay (VisPersist * psVisData) {
	struct tm * pTm;
	time_t ulTime;
	int nLoops;
	float fLength;
	float fVolume;
	char szString[NUMBER_STRING_MAX];

	glDisable (GL_LIGHTING);
	if (psVisData->boClearWhite) {
		glColor3f (0.0, 0.0, 0.0);
	}
	else {
		glColor3f (1.0, 1.0, 1.0);
	}
	ulTime = (unsigned long)psVisData->fCurrentTime;
	pTm = gmtime (& ulTime);
	RenderBitmapString (2.0, psVisData->nScreenHeight - LINEHEIGHT, GLUT_BITMAP_HELVETICA_10, asctime (pTm));

	nLoops = GetLoops (psVisData->psCelticData);
	snprintf (szString, NUMBER_STRING_MAX, "Loops:  %d", nLoops);
	RenderBitmapString (2.0, psVisData->nScreenHeight - (2 * LINEHEIGHT), GLUT_BITMAP_HELVETICA_10, szString);

	fLength = GetLength (psVisData->psCelticData);
	snprintf (szString, NUMBER_STRING_MAX, "Length: %f", fLength);
	RenderBitmapString (2.0, psVisData->nScreenHeight - (3 * LINEHEIGHT), GLUT_BITMAP_HELVETICA_10, szString);

	fVolume = GetVolume (psVisData->psCelticData);
	snprintf (szString, NUMBER_STRING_MAX, "Volume: %f", fVolume);
	RenderBitmapString (2.0, psVisData->nScreenHeight - (4 * LINEHEIGHT), GLUT_BITMAP_HELVETICA_10, szString);

	glEnable (GL_LIGHTING);
}

void WindowPos2f (GLfloat fX, GLfloat fY) {
	GLfloat fXn, fYn;

	glPushAttrib (GL_TRANSFORM_BIT | GL_VIEWPORT_BIT);
	glMatrixMode (GL_PROJECTION);
	glPushMatrix ();
	glLoadIdentity ();
	glMatrixMode (GL_MODELVIEW);
	glPushMatrix ();
	glLoadIdentity ();

	glDepthRange (0, 0);
	glViewport ((int)fX - 1, (int)fY - 1, 2, 2);
	fXn = fX - (int)fX;
	fYn = fY - (int)fY;
	glRasterPos4f (fXn, fYn, 0.0, 1);
	glPopMatrix ();
	glMatrixMode (GL_PROJECTION);
	glPopMatrix ();

	glPopAttrib ();
}

void RenderBitmapString (float fX, float fY, void * pFont, char const * szString) {
	int nPos;
	int nLine;

	nLine = 0;
	WindowPos2f (fX, fY);
	nPos = 0;
	while (szString[nPos] > 0) {
		glutBitmapCharacter (pFont, szString[nPos]);
		if (szString[nPos] == '\n') {
			nLine++;
			WindowPos2f (fX, fY - (nLine * 10));
		}
		nPos++;
	}
}

void Idle (VisPersist * psVisData) {
	struct timeb sTime;

	psVisData->fPrevTime = psVisData->fCurrentTime;
	ftime (& sTime);
	psVisData->fCurrentTime = (double)(sTime.time) + (double)(sTime.millitm) / 1000.0;
	if (psVisData->fCurrentTime < 0) psVisData->fCurrentTime = 0.0;

	KeyIdle (psVisData);

	MomentumSpin (psVisData);

	if (psVisData->boSpin) {
		Spin (psVisData);
	}
}

void Reshape (int nWidth, int nHeight, VisPersist * psVisData) {
	glMatrixMode (GL_PROJECTION);
	glLoadIdentity ();

	glViewport (0, 0, nWidth, nHeight);

	psVisData->nScreenWidth = nWidth;
	psVisData->nScreenHeight = nHeight;

	if (!psVisData->boFullScreen) {
		psVisData->nPrevScreenWidth = psVisData->nScreenWidth;
		psVisData->nPrevScreenHeight = psVisData->nScreenHeight;
	}

	gluPerspective (60, (float)nWidth / (float)nHeight, 1, 100);
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity ();
}

int GetScreenHeight (VisPersist * psVisData) {
	return psVisData->nScreenHeight;
}

int GetScreenWidth (VisPersist * psVisData) {
	return psVisData->nScreenWidth;
}

void Redraw (VisPersist * psVisData) {
	if (psVisData->boClearWhite) {
		glClearColor (1.0f, 1.0f, 1.0f, 0.0f);
	}
	else {
		glClearColor (0.0f, 0.0f, 0.0f, 0.0f);
	}
	glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix ();
	Render (psVisData);
	if (psVisData->boDebug) {
		DrawTextOverlay (psVisData);
	}
	glPopMatrix ();
}

void Mouse (int button, int state, int x, int y, VisPersist * psVisData) {
	if (button == LEFT_BUTTON && state == BUTTON_DOWN) {
		MouseDown (x, y, psVisData);
	}
	if (button == LEFT_BUTTON && state == BUTTON_UP) {
		// Mouse click up or finished drag
		switch (psVisData->eDrag) {
		default:
		case DRAG_VIEW:
			// Do nothing
			break;
		}
		psVisData->eDrag = DRAG_NONE;
		psVisData->fMomentum = 1.0f;
	}
}

void MouseDown (int nXPos, int nYPos, VisPersist * psVisData) {
	SelectInfo sSelectData;

	// Mouse click or start of drag
	psVisData->nXMouse = nXPos;
	psVisData->nYMouse = nYPos;
	psVisData->fMomentum = 0.0f;
	psVisData->fXMomentum = 0.0f;
	psVisData->fYMomentum = 0.0f;
	psVisData->fZMomentum = 0.0f;

	// Find out which object was selected
	sSelectData.nXPos = nXPos;
	sSelectData.nYPos = nYPos;
	SelectVisObject (& sSelectData, psVisData);

	psVisData->eSelectObject = sSelectData.eObjectType;

	switch (sSelectData.eObjectType) {
	case SELECTOBJECT_INVALID:
		psVisData->eDrag = DRAG_VIEW;
		break;
	default:
		// Do nothing;
		break;
	}
}

void MomentumSpin (VisPersist * psVisData) {
	if (psVisData->eDrag == DRAG_VIEW) {
		psVisData->fXMomentum = 0.0f;
		psVisData->fYMomentum = 0.0f;
		psVisData->fZMomentum = 0.0f;
	}
	if (psVisData->fMomentum > MOMENTUM_MIN) {
		ChangeView ((psVisData->fXMomentum * psVisData->fMomentum), (psVisData->fYMomentum * psVisData->fMomentum), (psVisData->fZMomentum * psVisData->fMomentum), 0.0f, psVisData);

		psVisData->fMomentum *= (pow (MOMENTUM_RESISTANCE, 100.0 * (psVisData->fCurrentTime - psVisData->fPrevTime)));
	}
}

void ToggleFullScreen (VisPersist * psVisData) {
	psVisData->boFullScreen = !psVisData->boFullScreen;
}

bool GetFullScreen (VisPersist * psVisData) {
	return psVisData->boFullScreen;
}

void ToggleClearWhite (VisPersist * psVisData) {
	psVisData->boClearWhite = !psVisData->boClearWhite;
}

void ToggleSpin (VisPersist * psVisData) {
	psVisData->boSpin = !psVisData->boSpin;
}

void SetClearWhite (bool boClearWhite, VisPersist * psVisData) {
	psVisData->boClearWhite = boClearWhite;
}

bool GetClearWhite (VisPersist * psVisData) {
	return psVisData->boClearWhite;
}

int GetDimensions (VisPersist * psVisData) {
	return psVisData->nDimensions;
}

void SetDimensions (int nDimensions, VisPersist * psVisData) {
	CelticPersist * psCelticData;

	if (nDimensions != psVisData->nDimensions) {
		switch (nDimensions) {
		case 2:
			psVisData->nDimensions = nDimensions;
			psCelticData = NewCelticPersist2D (3, 3, 3.0f, 3.0f, psVisData->psBezData);
			CopyCelticPersistParams (psVisData->psCelticData, psCelticData);
			DeleteCelticPersist (psVisData->psCelticData);
			psVisData->psCelticData = psCelticData;
			//psVisData->psCelticData = ConvertTo2D (psVisData->psCelticData);
			break;
		case 3:
			psVisData->nDimensions = nDimensions;
			psCelticData = NewCelticPersist3D (3, 4, 3, 3.0f, 3.0f, 3.0f, psVisData->psBezData);
			CopyCelticPersistParams (psVisData->psCelticData, psCelticData);
			DeleteCelticPersist (psVisData->psCelticData);
			psVisData->psCelticData = psCelticData;
			//psVisData->psCelticData = ConvertTo3D (psVisData->psCelticData);
			break;
		default:
			fprintf (stderr, "Knots with %d dimensions aren't supported.\n", nDimensions);
			break;
		}
	}
}

void GetDisplayProperties (float * pfViewRadius, float * pfTileX, float * pfTileY, float * pfTileZ, float * pfThickness, float * pfWeave, float * pfInsetX, float * pfInsetY, float * pfInsetZ, float * pfCurve, VisPersist * psVisData) {
	if (pfViewRadius) {
		* pfViewRadius = psVisData->fViewRadius;
	}

	if (pfTileX) {
		* pfTileX = GetTileX (psVisData->psCelticData);
	}

	if (pfTileY) {
		* pfTileY = GetTileY (psVisData->psCelticData);
	}

	if (pfTileZ) {
		* pfTileZ = GetTileZ (psVisData->psCelticData);
	}

	if (pfThickness) {
		* pfThickness = GetThickness (psVisData->psCelticData);
	}

	if (pfWeave) {
		* pfWeave = GetWeaveHeight (psVisData->psCelticData);
	}

	if (pfInsetX) {
		* pfInsetX = GetInsetX (psVisData->psCelticData);
	}

	if (pfInsetY) {
		* pfInsetY = GetInsetY (psVisData->psCelticData);
	}

	if (pfInsetZ) {
		* pfInsetZ = GetInsetZ (psVisData->psCelticData);
	}

	if (pfCurve) {
		* pfCurve = GetControlScale (psVisData->psCelticData);
	}
}

void SetDisplayProperties (float fViewRadius, float fTileX, float fTileY, float fTileZ, float fThickness, float fWeave, float fInsetX, float fInsetY, float fInsetZ, float fCurve, VisPersist * psVisData) {
	bool boRedraw;

	boRedraw = FALSE;
	psVisData->fViewRadius = fViewRadius;
	boRedraw |= SetTileX (fTileX, psVisData->psCelticData);
	boRedraw |= SetTileY (fTileY, psVisData->psCelticData);
	boRedraw |= SetTileZ (fTileZ, psVisData->psCelticData);
	boRedraw |= SetThickness (fThickness, psVisData->psCelticData);
	boRedraw |= SetWeaveHeight (fWeave, psVisData->psCelticData);
	boRedraw |= SetInsetX (fInsetX, psVisData->psCelticData);
	boRedraw |= SetInsetY (fInsetY, psVisData->psCelticData);
	boRedraw |= SetInsetZ (fInsetZ, psVisData->psCelticData);
	boRedraw |= SetControlScale (fCurve, psVisData->psCelticData);

	if (boRedraw) {
		RenderKnots (psVisData->psCelticData);
	}
}

unsigned int GetRandomSeed () {
	time_t nTimer;
	unsigned int uSeed;

	uSeed = (unsigned int)time (& nTimer);
	uSeed ^= (unsigned int)clock () / (CLOCKS_PER_SEC / 100);
	srand (uSeed);
	uSeed = rand ();

	return uSeed;
}

void Key (unsigned char key, int x, int y, unsigned int uKeyModifiers, VisPersist * psVisData) {
	psVisData->aboKeyDown[key] = true;
	unsigned int uSeed;

	switch (key) {
		case 'w':
			psVisData->boClearWhite = !psVisData->boClearWhite;
			break;
		case 'r':
			ResetAnimation (psVisData);
			break;
		case 'o':
			psVisData->boSpin = !psVisData->boSpin;
			break;
		case 'd':
			psVisData->boDebug = !psVisData->boDebug;
			break;
		case 'c':
			// New colour seed;
			uSeed = GetRandomSeed ();
			SetColourSeed (uSeed, psVisData->psCelticData);
			GenerateKnot (psVisData->psCelticData);
			RenderKnots (psVisData->psCelticData);
			break;
		case 'z':
			// Set colour seed to zero;
			SetColourSeed (0, psVisData->psCelticData);
			GenerateKnot (psVisData->psCelticData);
			RenderKnots (psVisData->psCelticData);
			break;
		case 'x':
			// New knot seed;
			uSeed = GetRandomSeed ();
			SetSeed (uSeed, psVisData->psCelticData);
			GenerateKnot (psVisData->psCelticData);
			RenderKnots (psVisData->psCelticData);
			break;
	}
}

void KeyUp (unsigned char key, int x, int y, unsigned int uKeyModifiers, VisPersist * psVisData) {
	psVisData->aboKeyDown[key] = false;

	switch (key) {
		case 's':
			// Do nothing
			break;
	}
}

void KeyIdle (VisPersist * psVisData) {
	if (psVisData->aboKeyDown['m']) ChangeView (0.0f, 0.0f, 0.0f, -RADIUSSTEP, psVisData);
	if (psVisData->aboKeyDown['n']) ChangeView (0.0f, 0.0f, 0.0f, +RADIUSSTEP, psVisData);
	if (psVisData->aboKeyDown[',']) ChangeView (0.0f, 0.0f, +PSISTEP, 0.0f, psVisData);
	if (psVisData->aboKeyDown['.']) ChangeView (0.0f, 0.0f, -PSISTEP, 0.0f, psVisData);
}

void ChangeView (float fTheta, float fPhi, float fPsi, float fRadius, VisPersist * psVisData) {
	float fA;
	float fB;
	float fX;
	float fY;
	float fZ;
	float fXn;
	float fYn;
	float fZn;

	float fXv;
	float fYv;
	float fZv;

	psVisData->fViewRadius += fRadius;

	// Phi
	fA = cos (fPhi);
	fB = sin (fPhi);

	fX = (fA * psVisData->fX) + (fB * psVisData->fXn);
	fY = (fA * psVisData->fY) + (fB * psVisData->fYn);
	fZ = (fA * psVisData->fZ) + (fB * psVisData->fZn);

	fXn = - (fB * psVisData->fX) + (fA * psVisData->fXn);
	fYn = - (fB * psVisData->fY) + (fA * psVisData->fYn);
	fZn = - (fB * psVisData->fZ) + (fA * psVisData->fZn);

	psVisData->fX = fX;
	psVisData->fY = fY;
	psVisData->fZ = fZ;

	psVisData->fXn = fXn;
	psVisData->fYn = fYn;
	psVisData->fZn = fZn;

	// Theta
	fXv = (psVisData->fY * psVisData->fZn) - (psVisData->fZ * psVisData->fYn);
	fYv = (psVisData->fZ * psVisData->fXn) - (psVisData->fX * psVisData->fZn);
	fZv = (psVisData->fX * psVisData->fYn) - (psVisData->fY * psVisData->fXn);

	fA = cos (fTheta);
	fB = sin (fTheta);

	fX = (fA * psVisData->fX) + (fB * fXv);
	fY = (fA * psVisData->fY) + (fB * fYv);
	fZ = (fA * psVisData->fZ) + (fB * fZv);

	psVisData->fX = fX;
	psVisData->fY = fY;
	psVisData->fZ = fZ;

	// Psi
	fA = cos (fPsi);
	fB = sin (fPsi);

	fXv = (psVisData->fY * psVisData->fZn) - (psVisData->fZ * psVisData->fYn);
	fYv = (psVisData->fZ * psVisData->fXn) - (psVisData->fX * psVisData->fZn);
	fZv = (psVisData->fX * psVisData->fYn) - (psVisData->fY * psVisData->fXn);

	fXn = (fA * fXn) - (fB * fXv);
	fYn = (fA * fYn) - (fB * fYv);
	fZn = (fA * fZn) - (fB * fZv);

	psVisData->fXn = fXn;
	psVisData->fYn = fYn;
	psVisData->fZn = fZn;

	// Normalise vectors (they should already be, but we make sure to avoid
	// cumulative rounding errors)

	Normalise3f (& psVisData->fX, & psVisData->fY, & psVisData->fZ);
	Normalise3f (& psVisData->fXn, & psVisData->fYn, & psVisData->fZn);
}

void Motion (int nX, int nY, VisPersist * psVisData) {
	switch (psVisData->eDrag) {
		case DRAG_VIEW:
			ChangeView ((nX - psVisData->nXMouse) / MOUSE_ROTATE_SCALE,
				(nY - psVisData->nYMouse) / MOUSE_ROTATE_SCALE, 0.0f, 0.0f, psVisData);

			psVisData->fXMomentum = ((float)nX - (float)psVisData->nXMouse) / MOUSE_ROTATE_SCALE;
			psVisData->fYMomentum = ((float)nY - (float)psVisData->nYMouse) / MOUSE_ROTATE_SCALE;
			psVisData->fZMomentum = 0.0f;

			psVisData->nXMouse = nX;
			psVisData->nYMouse = nY;
			break;
		default:
			// Do nothing
			break;
	}
}

gboolean GetMoving (VisPersist * psVisData) {
	return (psVisData->eDrag != DRAG_NONE);
}

void ResetAnimation (VisPersist * psVisData) {
	struct timeb sTime;

	// Reset the time
	ftime (& sTime);

	psVisData->fCurrentTime = (double)(sTime.time) + (double)(sTime.millitm) / 1000.0;
	if (psVisData->fCurrentTime < 0) psVisData->fCurrentTime = 0.0;
	psVisData->fPrevTime = psVisData->fCurrentTime;
	psVisData->fSpinTime = psVisData->fCurrentTime;

	psVisData->fX = 0.0f;
	psVisData->fY = 0.0f;
	psVisData->fZ = 1.0f;
	psVisData->fXn = 0.0f;
	psVisData->fYn = 1.0f;
	psVisData->fZn = 0.0f;

	psVisData->fMomentum = 0.0f;
	psVisData->fXMomentum = 0.0f;
	psVisData->fYMomentum = 0.0f;
	psVisData->fZMomentum = 0.0f;
}

void SaveSettingsVis (SettingsPersist * psSettingsData, VisPersist * psVisData) {
	SettingsPrintBool (psSettingsData, "FullScreen", psVisData->boFullScreen);
	SettingsPrintBool (psSettingsData, "ClearWhite", psVisData->boClearWhite);
	SettingsPrintBool (psSettingsData, "Debug", psVisData->boDebug);
	SettingsPrintFloat (psSettingsData, "ViewRadius", psVisData->fViewRadius);
	SettingsPrintFloat (psSettingsData, "ViewX", psVisData->fX);
	SettingsPrintFloat (psSettingsData, "ViewY", psVisData->fY);
	SettingsPrintFloat (psSettingsData, "ViewZ", psVisData->fZ);
	SettingsPrintFloat (psSettingsData, "ViewXNormal", psVisData->fXn);
	SettingsPrintFloat (psSettingsData, "ViewYNormal", psVisData->fYn);
	SettingsPrintFloat (psSettingsData, "ViewZNormal", psVisData->fZn);
	SettingsPrintInt (psSettingsData, "Dimensions", psVisData->nDimensions);

	SettingsStartTag (psSettingsData, "celtic");
	SaveSettingsCeltic (psSettingsData, psVisData->psCelticData);
	SettingsEndTag (psSettingsData, "celtic");
}

void LoadSettingsStartVis (SettingsPersist * psSettingsData, VisPersist * psVisData) {
	SettingsLoadParser * psLoadParser = NULL;

	psLoadParser = g_new0 (SettingsLoadParser, 1);

	psLoadParser->LoadProperty = VisLoadProperty;
	psLoadParser->LoadSectionStart = VisLoadSectionStart;
	psLoadParser->LoadSectionEnd = VisLoadSectionEnd;
	psLoadParser->psData = psVisData;
	AddParser (psLoadParser, psSettingsData);
}

void LoadSettingsEndVis (SettingsPersist * psSettingsData, VisPersist * psVisData) {
	SettingsLoadParser * psLoadParser = NULL;

	psLoadParser = GetParser (psSettingsData);
	g_free (psLoadParser);
	RemoveParser (psSettingsData);
}

void VisLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
	VisPersist * psVisData = (VisPersist *)psData;

	switch (eType) {
	case SETTINGTYPE_BOOL:
		if (stricmp (szName, "FullScreen") == 0) {
			psVisData->boFullScreen = *((bool*)(psValue));
		}
		else if (stricmp (szName, "ClearWhite") == 0) {
			psVisData->boClearWhite = *((bool*)(psValue));
		}
		else if (stricmp (szName, "Debug") == 0) {
			psVisData->boDebug = *((bool*)(psValue));
		}
	case SETTINGTYPE_FLOAT:
		if (stricmp (szName, "ViewRadius") == 0) {
			psVisData->fViewRadius = *((float*)(psValue));
		}
		else if (stricmp (szName, "ViewX") == 0) {
			psVisData->fX = *((float*)(psValue));
		}
		else if (stricmp (szName, "ViewY") == 0) {
			psVisData->fY = *((float*)(psValue));
		}
		else if (stricmp (szName, "ViewZ") == 0) {
			psVisData->fZ = *((float*)(psValue));
		}
		else if (stricmp (szName, "ViewXNormal") == 0) {
			psVisData->fXn = *((float*)(psValue));
		}
		else if (stricmp (szName, "ViewYNormal") == 0) {
			psVisData->fYn = *((float*)(psValue));
		}
		else if (stricmp (szName, "ViewZNormal") == 0) {
			psVisData->fZn = *((float*)(psValue));
		}
		break;
	case SETTINGTYPE_INT:
		if (stricmp (szName, "Dimensions") == 0) {
			SetDimensions (*((int*)(psValue)), psVisData);
			//psVisData->nDimensions = *((int*)(psValue));
		}
		break;
	default:
		printf ("Unknown vis property %s\n", szName);
		break;
	}
}

void VisLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {
	VisPersist * psVisData = (VisPersist *)psData;
	// Any subsections go in here
	if (stricmp (szName, "celtic") == 0) {
		// Move in to the vis section
		LoadSettingsStartCeltic (psSettingsData, psVisData->psCelticData);
	}
}

void VisLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
	VisPersist * psVisData = (VisPersist *)psData;

	if (stricmp (szName, "vis") == 0) {
		// Move out of the vis section
		LoadSettingsEndVis (psSettingsData, psVisData);
	}
}

CelticPersist * GetCelticData (VisPersist * psVisData) {
	return psVisData->psCelticData;
}

// Callback for freeing the pixel buffer data
void FreePixelBuffer (guchar * pixels, gpointer data) {
	// Free the data
	g_free (pixels);
}

bool ExportBitmap (char const * szFilename, char const * szType, int nHeight, int nWidth, VisPersist * psVisData) {
	bool boSuccess;
	int nScreenWidth;
	int nScreenHeight;
	GLuint uTexture;
	GLuint uRenderBuffer;
	GLuint uFrameBuffer;
	unsigned char * psData;
	GError * psError = NULL;
	GdkPixbuf * psImage;
	int nX;
	int nY;
	unsigned char cPixel;
	GLenum eSuccess;

	boSuccess = FALSE;

	nScreenWidth = psVisData->nScreenWidth;
	nScreenHeight = psVisData->nScreenHeight;
	Reshape (nWidth, nHeight, psVisData);

	// Create texture to store the bitmap knot image to
	glGenTextures (1, & uTexture);
	glBindTexture(GL_TEXTURE_2D, uTexture);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri (GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
	glBindTexture (GL_TEXTURE_2D, 0);

	// Create the render buffer to use as a depth buffer
	glGenRenderbuffers (1, & uRenderBuffer);
	glBindRenderbuffer (GL_RENDERBUFFER, uRenderBuffer);
	glRenderbufferStorage (GL_RENDERBUFFER, GL_DEPTH_COMPONENT, nWidth, nHeight);
	glBindRenderbuffer (GL_RENDERBUFFER, 0);

	// Create a framebuffer object to attach the texture and render buffer to
	glGenFramebuffers (1, & uFrameBuffer);
	glBindFramebuffer (GL_FRAMEBUFFER, uFrameBuffer);

	// Attach the texture (colour) and render buffer (depth)
	glFramebufferTexture2D (GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uTexture, 0);
	glFramebufferRenderbuffer (GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, uRenderBuffer);

	// Check if everything worked
	eSuccess = glCheckFramebufferStatus (GL_FRAMEBUFFER);
	if (eSuccess == GL_FRAMEBUFFER_COMPLETE) {
		// Use screen buffer
		glBindFramebuffer (GL_FRAMEBUFFER, 0);

		// Render to the framebuffer
		glBindFramebuffer (GL_FRAMEBUFFER, uFrameBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw the knot
		Redraw (psVisData);

		// Unbind frame buffer
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		// Bind the texture so we can extract the data and save it to disk
		glBindTexture (GL_TEXTURE_2D, uTexture);

		// Create some space to store the texture data in
		psData = g_new (unsigned char, (nHeight * nWidth * 4));

		// Extract the data from the texture
		glPixelStorei(GL_PACK_ALIGNMENT, 1);
		glGetTexImage (GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, psData);

		// We're done with the texture
		glBindTexture(GL_TEXTURE_2D, 0);

		// Flip the image
		for (nY = 0; nY < (nHeight / 2); nY++) {
			for (nX = 0; nX < (nWidth * 4); nX++) {
				cPixel = psData[(nY * nWidth * 4) + nX];
				psData[(nY * nWidth * 4) + nX] = psData[(((nHeight - 1) * nWidth * 4) - (nY * nWidth * 4)) + nX];
				psData[(((nHeight - 1) * nWidth * 4) - (nY * nWidth * 4)) + nX] = cPixel;
			}
		}

		// Convert the texture data into a Pixel Buffer object
		psImage = gdk_pixbuf_new_from_data (psData, GDK_COLORSPACE_RGB, TRUE, 8, nWidth, nHeight, nWidth * 4, FreePixelBuffer, NULL);
		if (psImage) {
			// Save the image to disk
			psError = NULL;
			gdk_pixbuf_save (psImage, szFilename, szType, & psError, NULL);
			if (psError) {
				printf ("%s", psError->message);
			}
			else {
				boSuccess = TRUE;
			}
		}
		g_object_unref (G_OBJECT (psImage));
		psData = NULL;
	}

	// Tidy things up
	glDeleteTextures (1, & uTexture);
	uTexture = 0;
	glDeleteFramebuffers (1, & uFrameBuffer);
	uFrameBuffer = 0;
	glDeleteRenderbuffers (1, & uRenderBuffer);
	uRenderBuffer = 0;

	// Ensure the screen image gets rendered at the correct size
	Reshape (nScreenWidth, nScreenHeight, psVisData);

	return boSuccess;
}


