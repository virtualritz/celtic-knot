///////////////////////////////////////////////////////////////////
// Bezier
// Render bezier tubes
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Summber 2011
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

#include "bez.h"

///////////////////////////////////////////////////////////////////
// Defines

#define true 								(1)
#define false 							(0)

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

#ifdef WORDS_BIGENDIAN
#define PLY_ENDIANNESS "binary_big_endian"
#else
#define PLY_ENDIANNESS "binary_little_endian"
#endif

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _BezStore BezStore;
typedef struct _BezDetails BezDetails;

struct _Bezier {
	GLuint * auVertexBuffer;
	Bezier * psNext;
	Bezier * psPrev;
};

struct _BezPersist {
	int nPieces;
	int nSegments;
	GLuint uStartList;
	Bezier * psBezierFirst;
	Bezier * psBezierLast;
	unsigned int uBeziers;
	GLuint * auIndexBuffer;
	BezStore * psBezierStore;
};

typedef struct _KnotVertex {
	float fX;
	float fY;
	float fZ;
	float fNX;
	float fNY;
	float fNZ;
	float afColour[BEZ_COL_COMPONENTS];
	//float fPadding[2];
} KnotVertex;

struct _BezDetails {
	Vector3 vStart;
	Vector3 vEnd;
	Vector3 vStartDir;
	Vector3 vEndDir;
	float fRadius;
	float afColourStart[BEZ_COL_COMPONENTS];
	float afColourEnd[BEZ_COL_COMPONENTS];
	BezDetails * psNext;
};

struct _BezStore {
	bool boStore;
	BezDetails * psBezierFirst;
	BezDetails * psBezierLast;
	int nBezierNum;
};

///////////////////////////////////////////////////////////////////
// Global variables

static GLfloat gafDiffuse[] = { 0.5, 0.5, 1.0, 1.0 };
static GLfloat gafSpecular[] = { 0.7, 0.7, 0.7, 1.0 };
static GLfloat gafShininess[] = { 10.0 };

///////////////////////////////////////////////////////////////////
// Function prototypes

void DrawBezier (GLdouble fXCentre, GLdouble fYCentre, GLdouble fZCentre, Bezier * psBezier, BezPersist * psBezData);
void ConvertTubeToBezier (KnotVertex * asVertex, int nPieces, int nSegments, float fRadius, Vector3 vStart, Vector3 vStartDir, Vector3 vEnd, Vector3 vEndDir, float const * afStartCol, float const * afEndCol);
Matrix3 CreateRotationMatrix (Vector3 * pvNormal);
GLuint * CreateBezierVertexBuffers (int nPieces, int nSegments);
GLuint * CreateBezierIndexBuffers (int nPieces, int nSegments);
BezStore * NewBezStore ();
void DeleteBezStore (BezStore * psBezStore);
void StoreBezierDetails (Vector3 const * pvStart, Vector3 const * pvStartDir, Vector3 const * pvEnd, Vector3 const * pvEndDir, float fRadius, float const * afColourStart, float const* afColourEnd, BezStore * psBezStore);
void ClearStoredBezierDetails (BezStore * psBezStore);
int OutputStoredVertices (FILE * hFile, int nPieces, int nSegments, BezDetails const * psBezDetails, bool boBinary);
int OutputStoredIndices (FILE * hFile, int nPieces, int nSegments, int nOffset, BezDetails const * psBezDetails, bool boBinary);

///////////////////////////////////////////////////////////////////
// Function definitions

BezPersist * NewBezPersist (int nPieces, int nSegments) {
	BezPersist * psBezData;

	psBezData = g_new0 (BezPersist, 1);
	
	psBezData->psBezierFirst = NULL;
	psBezData->psBezierLast = NULL;
	psBezData->uBeziers = 0u;

	// Initialise the index buffer
	psBezData->nPieces = nPieces;
	psBezData->nSegments = nSegments;
	psBezData->auIndexBuffer = CreateBezierIndexBuffers (psBezData->nPieces, psBezData->nSegments);
	psBezData->uStartList = 0;
	psBezData->psBezierStore = NewBezStore ();

	return psBezData;
}

void DeleteBezPersist (BezPersist * psBezData) {
	// Free the beziers
	while (psBezData->psBezierLast) {
		DeleteBezier (psBezData->psBezierLast, psBezData);
	}
	
	// Free the index buffers
	glDeleteBuffers (psBezData->nPieces, psBezData->auIndexBuffer);
	g_free (psBezData->auIndexBuffer);
	psBezData->auIndexBuffer = NULL;

	// Free the bezier store
	DeleteBezStore (psBezData->psBezierStore);

	g_free (psBezData);
}

Bezier * NewBezier (BezPersist * psBezData) {
	Bezier * psBezier;

	g_assert (psBezData);

	psBezier = g_new0 (Bezier, 1);
	
	// Link the structure into the linked list
	psBezier->psNext = NULL;
	psBezier->psPrev = psBezData->psBezierLast;
	if (psBezData->psBezierLast) {
		// The list has items, so relink the last one
		psBezData->psBezierLast->psNext = psBezier;
		psBezData->psBezierLast = psBezier;
	}
	else {
		// The list is currently empty
		psBezData->psBezierFirst = psBezier;
		psBezData->psBezierLast = psBezier;
	}

	// Initialise the vertex buffer
	psBezier->auVertexBuffer = CreateBezierVertexBuffers (psBezData->nPieces, psBezData->nSegments);

	psBezData->uBeziers++;

	return psBezier;
}

void DeleteBezier (Bezier * psBezier, BezPersist * psBezData) {
	// Free the vertex buffers
	glDeleteBuffers (1, psBezier->auVertexBuffer);
	free (psBezier->auVertexBuffer);
	psBezier->auVertexBuffer = NULL;

	if (psBezier->psPrev) {
		psBezier->psPrev->psNext = psBezier->psNext;
	}
	else {
		psBezData->psBezierFirst = psBezier->psNext;
	}

	if (psBezier->psNext) {
		psBezier->psNext->psPrev = psBezier->psPrev;
	}
	else {
		psBezData->psBezierLast = psBezier->psPrev;
	}
	g_free (psBezier);	

	psBezData->uBeziers--;
}

BezStore * NewBezStore () {
	BezStore * psBezStore;

	psBezStore = g_new0 (BezStore, 1);

	psBezStore->boStore = FALSE;
	psBezStore->psBezierFirst = NULL;
	psBezStore->psBezierLast = NULL;
	psBezStore->nBezierNum = 0;

	return psBezStore;
}

void DeleteBezStore (BezStore * psBezStore) {
	ClearStoredBezierDetails (psBezStore);

	g_free (psBezStore);
}

void StoreBezierDetails (Vector3 const * pvStart, Vector3 const * pvStartDir, Vector3 const * pvEnd, Vector3 const * pvEndDir, float fRadius, float const * afColourStart, float const * afColourEnd, BezStore * psBezStore) {
	BezDetails * psBezDetails;
	int nComponent;

	psBezDetails = g_new0 (BezDetails, 1);

	psBezDetails->vStart = * pvStart;
	psBezDetails->vEnd = * pvEnd;
	psBezDetails->vStartDir = * pvStartDir;
	psBezDetails->vEndDir = * pvEndDir;
	psBezDetails->fRadius = fRadius;
	if (afColourStart && afColourEnd) {
		for (nComponent = 0; nComponent < BEZ_COL_COMPONENTS; nComponent++) {
			psBezDetails->afColourStart[nComponent] = afColourStart[nComponent];
			psBezDetails->afColourEnd[nComponent] = afColourEnd[nComponent];
		}
	}
	else {
		for (nComponent = 0; nComponent < BEZ_COL_COMPONENTS; nComponent++) {
			psBezDetails->afColourStart[nComponent] = 0.5f;
			psBezDetails->afColourEnd[nComponent] = 0.5f;
		}
	}
	psBezDetails->psNext = NULL;

	if (psBezStore->psBezierFirst == NULL) {
		psBezStore->psBezierFirst = psBezDetails;
	}

	if (psBezStore->psBezierLast != NULL) {
		psBezStore->psBezierLast->psNext = psBezDetails;
	}
	psBezStore->psBezierLast = psBezDetails;
	psBezStore->nBezierNum++;
}

void ClearStoredBezierDetails (BezStore * psBezStore) {
	BezDetails * psBezDetails;
	BezDetails * psBezDetailsNext;

	// Free all of the bezier details	
	psBezDetails = psBezStore->psBezierFirst;
	while (psBezDetails) {
		psBezDetailsNext = psBezDetails->psNext;
		g_free (psBezDetails);
		psBezDetails = psBezDetailsNext;
	}

	psBezStore->psBezierFirst = NULL;
	psBezStore->psBezierLast = NULL;
	psBezStore->nBezierNum = 0;
}

void StoreBeziers (bool boStore, BezPersist * psBezData) {
	psBezData->psBezierStore->boStore = boStore;
	
	if (boStore == FALSE) {
		ClearStoredBezierDetails (psBezData->psBezierStore);
	}
}

bool OutputStoredBeziers (char const * szFilename, BezPersist * psBezData, bool boBinary) {
	bool boResult;
	int nVertices;
	int nFaces;
	FILE * hFile;
	BezDetails * psBezDetails;
	int nOffset;
	int nSegments;
	int nPieces;

	nSegments = psBezData->nSegments;
	nPieces = psBezData->nPieces;

	nVertices = ((nPieces + 1) * nSegments) * psBezData->psBezierStore->nBezierNum;
	nFaces = (2 * nPieces * nSegments) * psBezData->psBezierStore->nBezierNum;

	boResult = FALSE;
	hFile = fopen (szFilename, "w");
	if (hFile) {
		fprintf (hFile, "ply\n");
		if (boBinary) {
			fprintf (hFile, "format " PLY_ENDIANNESS " 1.0 1.0\n");
		}
		else {
			fprintf (hFile, "format ascii 1.0\n");
		}
		fprintf (hFile, "comment CelticKnot generated\n");
		fprintf (hFile, "element vertex %d\n", nVertices);
		fprintf (hFile, "property float x\n");
		fprintf (hFile, "property float y\n");
		fprintf (hFile, "property float z\n");
		fprintf (hFile, "property uint8 red\n");
		fprintf (hFile, "property uint8 green\n");
		fprintf (hFile, "property uint8 blue\n");
		fprintf (hFile, "element face %d\n", nFaces);
		fprintf (hFile, "property list uchar int vertex_indices\n");
		fprintf (hFile, "end_header\n");

		// Output the vertices
		psBezDetails = psBezData->psBezierStore->psBezierFirst;
		while (psBezDetails) {
			OutputStoredVertices (hFile, nPieces, nSegments, psBezDetails, boBinary);
			psBezDetails = psBezDetails->psNext;
		}
		
		// Output the indices
		psBezDetails = psBezData->psBezierStore->psBezierFirst;
		nOffset = 0;
		while (psBezDetails) {
			OutputStoredIndices (hFile, nPieces, nSegments, nOffset, psBezDetails, boBinary);
			nOffset += ((nPieces + 1) * nSegments);
			psBezDetails = psBezDetails->psNext;
		}

		fclose (hFile);
		boResult = TRUE;
	}

	return boResult;
}

int OutputStoredVertices (FILE * hFile, int nPieces, int nSegments, BezDetails const * psBezDetails, bool boBinary) {
	int nPiece;
	int nSegment;
	Vector3 vPos;
	Vector3 vOffset;
	float fStep;
	float fTheta;
	Vector3 vNormalPrev;
	Vector3 vNormal;
	Matrix3 mRotate;
	Matrix3 mRotation;
	Vector3 vVertex;
	int nVertices;
	float fColour;
  unsigned char ucColour[BEZ_COL_COMPONENTS];
  float fColScale;
  int nColComponent;

	vNormal.fX = 1.0f;
	vNormal.fY = 0.0f;
	vNormal.fZ = 0.0f;
	SetIdentity (& mRotation);

	nVertices = 0;
	for (nPiece = 0; nPiece < nPieces; nPiece++) {
		// Calculate the translation due to the bezier curve 
		fStep = ((float)(nPiece)) / ((float)(nPieces));
		vPos.fX = BEZIER (psBezDetails->vStart.fX, psBezDetails->vStartDir.fX, psBezDetails->vEnd.fX, psBezDetails->vEndDir.fX, fStep);
		vPos.fY = BEZIER (psBezDetails->vStart.fY, psBezDetails->vStartDir.fY, psBezDetails->vEnd.fY, psBezDetails->vEndDir.fY, fStep);
		vPos.fZ = BEZIER (psBezDetails->vStart.fZ, psBezDetails->vStartDir.fZ, psBezDetails->vEnd.fZ, psBezDetails->vEndDir.fZ, fStep);

		// Calculate the rotation due to the curve direction
		vNormalPrev = vNormal;
		vNormal.fX = BEZIERDERIV (psBezDetails->vStart.fX, psBezDetails->vStartDir.fX, psBezDetails->vEnd.fX, psBezDetails->vEndDir.fX, fStep);
		vNormal.fY = BEZIERDERIV (psBezDetails->vStart.fY, psBezDetails->vStartDir.fY, psBezDetails->vEnd.fY, psBezDetails->vEndDir.fY, fStep);
		vNormal.fZ = BEZIERDERIV (psBezDetails->vStart.fZ, psBezDetails->vStartDir.fZ, psBezDetails->vEnd.fZ, psBezDetails->vEndDir.fZ, fStep);

		mRotate = RotationBetweenVectors (& vNormalPrev, & vNormal);
		mRotation = MultMatrixMatrix (& mRotation, & mRotate);

		// Calculate vertex colour
		fColScale = ((float)nPiece / (float)nPieces);
		for (nColComponent = 0; nColComponent < BEZ_COL_COMPONENTS; nColComponent++) {
			fColour = (psBezDetails->afColourEnd[nColComponent] * fColScale) + (psBezDetails->afColourStart[nColComponent] * (1.0f - fColScale));
			ucColour[nColComponent] = (unsigned char)(fColour * 255.0);
		}

		for (nSegment = 0; nSegment < nSegments; nSegment++) {
			fTheta = (float)nSegment * (2.0 * M_PI / (float)nSegments);

			vOffset.fX = 0.0f;
			vOffset.fY = sin (fTheta);
			vOffset.fZ = cos (fTheta);

			// Apply the rotation
			vOffset = MultMatrixVector (& mRotation, & vOffset);

			vVertex.fX = vPos.fX + (psBezDetails->fRadius * vOffset.fX);
			vVertex.fY = vPos.fY + (psBezDetails->fRadius * vOffset.fY);
			vVertex.fZ = vPos.fZ + (psBezDetails->fRadius * vOffset.fZ);

			if (boBinary) {
				fwrite (& vVertex, sizeof (float), 3, hFile);
				fwrite (ucColour, sizeof (unsigned char), BEZ_COL_COMPONENTS, hFile);
			}
			else {
				fprintf (hFile, "%f %f %f\n", vVertex.fX, vVertex.fY, vVertex.fZ);
				fprintf (hFile, "%u %u %u ", ucColour[0], ucColour[1], ucColour[2]);
			}
			nVertices++;
		}
	}

	// We need to do something different for the last piece
	fStep = 1.0f;

	float fAngle;
	Vector3 vUnit1;
	Vector3 vUnit2;
	Vector3 vTwist1;
	Vector3 vTwist2;
	Vector3 vCross;
	Vector3 vSum;
	float fLength;

	SetVector3 (vUnit1, 1.0f, 0.0f, 0.0f);
	SetVector3 (vUnit2, 0.0f, 1.0f, 0.0f);

	vPos.fX = BEZIER (psBezDetails->vStart.fX, psBezDetails->vStartDir.fX, psBezDetails->vEnd.fX, psBezDetails->vEndDir.fX, fStep);
	vPos.fY = BEZIER (psBezDetails->vStart.fY, psBezDetails->vStartDir.fY, psBezDetails->vEnd.fY, psBezDetails->vEndDir.fY, fStep);
	vPos.fZ = BEZIER (psBezDetails->vStart.fZ, psBezDetails->vStartDir.fZ, psBezDetails->vEnd.fZ, psBezDetails->vEndDir.fZ, fStep);

	vNormalPrev = vNormal;
	vNormal.fX = BEZIERDERIV (psBezDetails->vStart.fX, psBezDetails->vStartDir.fX, psBezDetails->vEnd.fX, psBezDetails->vEndDir.fX, fStep);
	vNormal.fY = BEZIERDERIV (psBezDetails->vStart.fY, psBezDetails->vStartDir.fY, psBezDetails->vEnd.fY, psBezDetails->vEndDir.fY, fStep);
	vNormal.fZ = BEZIERDERIV (psBezDetails->vStart.fZ, psBezDetails->vStartDir.fZ, psBezDetails->vEnd.fZ, psBezDetails->vEndDir.fZ, fStep);

	mRotate = RotationBetweenVectors (& vNormalPrev, & vNormal);
	mRotation = MultMatrixMatrix (& mRotation, & mRotate);
	vTwist1 = MultMatrixVector (& mRotation, & vUnit2);

	mRotate = RotationBetweenVectors (& vUnit1, & vNormal);
	vTwist2 = MultMatrixVector (& mRotate, & vUnit2);

	fAngle = DotProdAngleVector (& vTwist1, & vTwist2);
	fAngle = fmodf (fAngle, (2.0f * M_PI / (float)nSegments));
	if (fAngle > (M_PI / (float)nSegments)) {
		fAngle = fAngle - (2.0f * M_PI / (float)nSegments);
	}

	if (fabs (fAngle) > 0.0f) {
		vCross = CrossProduct (& vTwist1, & vTwist2);
		Normalise (& vCross);
		vSum = AddVectors (& vCross, & vNormal);
		fLength = Length (& vSum);
		if (fLength > 1.0f) {
			fAngle = -fAngle;
		}
	}
	
	// Calculate vertex colour
	fColScale = ((float)nPiece / (float)nPieces);
	for (nColComponent = 0; nColComponent < BEZ_COL_COMPONENTS; nColComponent++) {
		fColour = (psBezDetails->afColourEnd[nColComponent] * fColScale) + (psBezDetails->afColourStart[nColComponent] * (1.0f - fColScale));
		ucColour[nColComponent] = (unsigned char)(fColour * 255.0);
	}

	for (nSegment = 0; nSegment < nSegments; nSegment++) {
		fTheta = (float)nSegment * (2.0 * M_PI / (float)nSegments);
		fTheta += fAngle;


		vOffset.fX = 0.0f;
		vOffset.fY = sin (fTheta);
		vOffset.fZ = cos (fTheta);

		// Apply the rotation
		vOffset = MultMatrixVector (& mRotation, & vOffset);
		//vOffset = MultMatrixVector (& mRotate, & vOffset);


		vVertex.fX = vPos.fX + (psBezDetails->fRadius * vOffset.fX);
		vVertex.fY = vPos.fY + (psBezDetails->fRadius * vOffset.fY);
		vVertex.fZ = vPos.fZ + (psBezDetails->fRadius * vOffset.fZ);

		if (boBinary) {
			fwrite (& vVertex, sizeof (float), 3, hFile);
			fwrite (ucColour, sizeof (unsigned char), BEZ_COL_COMPONENTS, hFile);
		}
		else {
			fprintf (hFile, "%f %f %f\n", vVertex.fX, vVertex.fY, vVertex.fZ);
			fprintf (hFile, "%u %u %u ", ucColour[0], ucColour[1], ucColour[2]);
		}
		nVertices++;
	}
	
	return nVertices;
}

int OutputStoredIndices (FILE * hFile, int nPieces, int nSegments, int nOffset, BezDetails const * psBezDetails, bool boBinary) {
	int nPiece;
	int nSegment;
	int nIndices;
	int anIndex[3];
	unsigned char uVertices;
	
	// This function outputs  index buffer identifiers
	uVertices = 3;
	nIndices = 0;
	for (nPiece = 0; nPiece < nPieces; nPiece++) {
		for (nSegment = 0; nSegment < nSegments; nSegment++) {
			anIndex[0] = ((nPiece + 0) * nSegments) + ((nSegment + 0) % nSegments) + nOffset;
			anIndex[1] = ((nPiece + 1) * nSegments) + ((nSegment + 0) % nSegments) + nOffset;
			anIndex[2] = ((nPiece + 0) * nSegments) + ((nSegment + 1) % nSegments) + nOffset;

			if (boBinary) {
				fwrite (& uVertices, sizeof (unsigned char), 1, hFile);
				fwrite (& anIndex, sizeof (int), 3, hFile);
			}
			else {
				fprintf (hFile, "3 %d %d %d\n", anIndex[0], anIndex[1], anIndex[2]);
			}
			nIndices++;

			anIndex[0] = ((nPiece + 0) * nSegments) + ((nSegment + 1) % nSegments) + nOffset;
			anIndex[1] = ((nPiece + 1) * nSegments) + ((nSegment + 0) % nSegments) + nOffset;
			anIndex[2] = ((nPiece + 1) * nSegments) + ((nSegment + 1) % nSegments) + nOffset;

			if (boBinary) {
				fwrite (& uVertices, sizeof (unsigned char), 1, hFile);
				fwrite (& anIndex, sizeof (int), 3, hFile);
			}
			else {
				fprintf (hFile, "3 %d %d %d\n", anIndex[0], anIndex[1], anIndex[2]);
			}
			nIndices++;
		}
	}
	
	return nIndices;
}

GLuint * CreateBezierVertexBuffers (int nPieces, int nSegments) {
	KnotVertex * asVertex = NULL;
	int nPiece;
	int nSegment;
	int nVertices;
	float fTheta;
	float fRadius;
	float fLength;
	float fX;
	float fY;
	float fZ;
	int nIndex;
	GLuint * auVertexBuffer;

	// This function generates an array of vertex buffer identifiers
	// Once they're no longer needed, these vertex buffers should be released and the array freed
	auVertexBuffer = g_new0 (GLuint, 1);

	fLength = 5.0f;
	fRadius = 0.25f;

	// Vertex buffers
	glGenBuffers (1, auVertexBuffer);
	g_assert (auVertexBuffer[0] > 0);
	glBindBuffer (GL_ARRAY_BUFFER, auVertexBuffer[0]);
	nVertices = (nPieces + 1) * nSegments;
	glBufferData (GL_ARRAY_BUFFER, nVertices * sizeof (KnotVertex), NULL, GL_DYNAMIC_DRAW);
	asVertex = g_malloc (nVertices * sizeof (KnotVertex));

	for (nPiece = 0; nPiece <= nPieces; nPiece++) {
		for (nSegment = 0; nSegment < nSegments; nSegment++) {
			nIndex = (nPiece * nSegments) + nSegment;
			fTheta = (float)nSegment * (2.0 * M_PI / (float)nSegments);
			fX = (float)nPiece * (fLength / (float)nPieces);
			fY = fRadius * sin (fTheta);
			fZ = fRadius * cos (fTheta);

			asVertex[nIndex].fX = fX;
			asVertex[nIndex].fY = fY;
			asVertex[nIndex].fZ = fZ;
			asVertex[nIndex].fNX = 0.0f;
			asVertex[nIndex].fNY = sin (fTheta);
			asVertex[nIndex].fNZ = cos (fTheta);

			asVertex[nIndex].afColour[0] = 1.0f;
			asVertex[nIndex].afColour[1] = 0.5f;
			asVertex[nIndex].afColour[2] = 1.0f;
		}
	}

	glBufferSubData (GL_ARRAY_BUFFER, 0, nVertices * sizeof (KnotVertex), asVertex);
	free (asVertex);
	asVertex = NULL;
	
	return auVertexBuffer;
}

void SetBezierControlPoints (float fRadius, Vector3 vStart, Vector3 vStartDir, Vector3 vEnd, Vector3 vEndDir, float const * afStartCol, float const * afEndCol, Bezier * psBezier, BezPersist * psBezData) {
	void * psBuffer;
	float const afDefaultCol[3] = {0.5f, 0.5f, 1.0f};
	float const * pfStartCol;
	float const * pfEndCol;

	glBindBuffer (GL_ARRAY_BUFFER, psBezier->auVertexBuffer[0]);
	psBuffer = glMapBuffer (GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	pfStartCol = (afStartCol ? afStartCol : afDefaultCol);
	pfEndCol = (afEndCol ? afEndCol : afDefaultCol);

	ConvertTubeToBezier (psBuffer, psBezData->nPieces, psBezData->nSegments, fRadius, vStart, vStartDir, vEnd, vEndDir, pfStartCol, pfEndCol);

	glUnmapBuffer (GL_ARRAY_BUFFER);

	if ((psBezData->psBezierStore->boStore) && afStartCol && afEndCol) {
		StoreBezierDetails (& vStart, & vStartDir, & vEnd, & vEndDir, fRadius, afStartCol, afEndCol, psBezData->psBezierStore);
	}
}

GLuint * CreateBezierIndexBuffers (int nPieces, int nSegments) {
	unsigned short * auIndex = NULL;
	int nPiece;
	int nSegment;
	GLuint * auIndexBuffer;
	int nIndices;
	int nIndex;
	
	// This function generates an array of index buffer identifiers
	// Once they're no longer needed, these index buffers should be released and the array freed
	auIndexBuffer = malloc (nPieces * sizeof (GLuint));

	// Index buffers
	glGenBuffers (nPieces, auIndexBuffer);
	nIndices = (nSegments + 1) * 2;
	auIndex = g_new0 (unsigned short, nIndices);

	for (nPiece = 0; nPiece < nPieces; nPiece++) {
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, auIndexBuffer[nPiece]);
		glBufferData (GL_ELEMENT_ARRAY_BUFFER, nIndices * sizeof (unsigned short), NULL, GL_STATIC_DRAW);
		for (nSegment = 0; nSegment <= nSegments; nSegment++) {
			nIndex = (nSegment * 2);
			auIndex[(nIndex + 0)] = ((nPiece + 0) * nSegments) + (nSegment % nSegments);
			auIndex[(nIndex + 1)] = ((nPiece + 1) * nSegments) + (nSegment % nSegments);
		}
		glBufferSubData (GL_ELEMENT_ARRAY_BUFFER, 0, nIndices * sizeof (unsigned short), auIndex);
	}

	free (auIndex);
	auIndex = NULL;
	
	return auIndexBuffer;
}


void ConvertTubeToBezier (KnotVertex * asVertex, int nPieces, int nSegments, float fRadius, Vector3 vStart, Vector3 vStartDir, Vector3 vEnd, Vector3 vEndDir, float const * afStartCol, float const * afEndCol) {
	int nPiece;
	int nSegment;
	Vector3 vPos;
	Vector3 vOffset;
	float fStep;
	float fTheta;
	int nIndex;
	Vector3 vNormalPrev;
	Vector3 vNormal;
	Matrix3 mRotate;
	Matrix3 mRotation;
	float fColScale;
	float fAngle;
	Vector3 vUnit1;
	Vector3 vUnit2;
	Vector3 vTwist1;
	Vector3 vTwist2;
	Vector3 vCross;
	Vector3 vSum;
	float fLength;


	SetVector3 (vNormal, 1.0f, 0.0f, 0.0f);
	SetIdentity (& mRotation);

	for (nPiece = 0; nPiece < nPieces; nPiece++) {
		fColScale = ((float)nPiece / (float)nPieces);

		// Calculate the translation due to the bezier curve 
		fStep = ((float)(nPiece)) / ((float)(nPieces));
		vPos.fX = BEZIER (vStart.fX, vStartDir.fX, vEnd.fX, vEndDir.fX, fStep);
		vPos.fY = BEZIER (vStart.fY, vStartDir.fY, vEnd.fY, vEndDir.fY, fStep);
		vPos.fZ = BEZIER (vStart.fZ, vStartDir.fZ, vEnd.fZ, vEndDir.fZ, fStep);

		// Calculate the rotation due to the curve direction
		vNormalPrev = vNormal;
		vNormal.fX = BEZIERDERIV (vStart.fX, vStartDir.fX, vEnd.fX, vEndDir.fX, fStep);
		vNormal.fY = BEZIERDERIV (vStart.fY, vStartDir.fY, vEnd.fY, vEndDir.fY, fStep);
		vNormal.fZ = BEZIERDERIV (vStart.fZ, vStartDir.fZ, vEnd.fZ, vEndDir.fZ, fStep);

		mRotate = RotationBetweenVectors (& vNormalPrev, & vNormal);
		mRotation = MultMatrixMatrix (& mRotation, & mRotate);

		for (nSegment = 0; nSegment < nSegments; nSegment++) {
			nIndex = (nPiece * nSegments) + nSegment;
			fTheta = (float)nSegment * (2.0 * M_PI / (float)nSegments);

			vOffset.fX = 0.0f;
			vOffset.fY = sin (fTheta);
			vOffset.fZ = cos (fTheta);

			// Apply the rotation
			vOffset = MultMatrixVector (& mRotation, & vOffset);

			asVertex[nIndex].fX = vPos.fX + (fRadius * vOffset.fX);
			asVertex[nIndex].fY = vPos.fY + (fRadius * vOffset.fY);
			asVertex[nIndex].fZ = vPos.fZ + (fRadius * vOffset.fZ);
			asVertex[nIndex].fNX = vOffset.fX;
			asVertex[nIndex].fNY = vOffset.fY;
			asVertex[nIndex].fNZ = vOffset.fZ;

			asVertex[nIndex].afColour[0] = (afEndCol[0] * fColScale) + (afStartCol[0] * (1.0f - fColScale));
			asVertex[nIndex].afColour[1] = (afEndCol[1] * fColScale) + (afStartCol[1] * (1.0f - fColScale));
			asVertex[nIndex].afColour[2] = (afEndCol[2] * fColScale) + (afStartCol[2] * (1.0f - fColScale));
		}
	}

	// We need to do something different for the last piece
	fStep = 1.0f;

	SetVector3 (vUnit1, 1.0f, 0.0f, 0.0f);
	SetVector3 (vUnit2, 0.0f, 1.0f, 0.0f);

	fColScale = 1.0f;
	vPos.fX = BEZIER (vStart.fX, vStartDir.fX, vEnd.fX, vEndDir.fX, fStep);
	vPos.fY = BEZIER (vStart.fY, vStartDir.fY, vEnd.fY, vEndDir.fY, fStep);
	vPos.fZ = BEZIER (vStart.fZ, vStartDir.fZ, vEnd.fZ, vEndDir.fZ, fStep);

	vNormalPrev = vNormal;
	vNormal.fX = BEZIERDERIV (vStart.fX, vStartDir.fX, vEnd.fX, vEndDir.fX, fStep);
	vNormal.fY = BEZIERDERIV (vStart.fY, vStartDir.fY, vEnd.fY, vEndDir.fY, fStep);
	vNormal.fZ = BEZIERDERIV (vStart.fZ, vStartDir.fZ, vEnd.fZ, vEndDir.fZ, fStep);

	mRotate = RotationBetweenVectors (& vNormalPrev, & vNormal);
	mRotation = MultMatrixMatrix (& mRotation, & mRotate);
	vTwist1 = MultMatrixVector (& mRotation, & vUnit2);

	mRotate = RotationBetweenVectors (& vUnit1, & vNormal);
	vTwist2 = MultMatrixVector (& mRotate, & vUnit2);

	fAngle = DotProdAngleVector (& vTwist1, & vTwist2);
	fAngle = fmodf (fAngle, (2.0f * M_PI / (float)nSegments));
	if (fAngle > (M_PI / (float)nSegments)) {
		fAngle = fAngle - (2.0f * M_PI / (float)nSegments);
	}

	if (fabs (fAngle) > 0.0f) {
		vCross = CrossProduct (& vTwist1, & vTwist2);
		Normalise (& vCross);
		vSum = AddVectors (& vCross, & vNormal);
		fLength = Length (& vSum);
		if (fLength > 1.0f) {
			fAngle = -fAngle;
		}
	}
	
	for (nSegment = 0; nSegment < nSegments; nSegment++) {
		nIndex = (nPiece * nSegments) + nSegment;
		fTheta = (float)nSegment * (2.0 * M_PI / (float)nSegments);
		fTheta += fAngle;

		vOffset.fX = 0.0f;
		vOffset.fY = sin (fTheta);
		vOffset.fZ = cos (fTheta);

		// Apply the rotation
		vOffset = MultMatrixVector (& mRotation, & vOffset);

		asVertex[nIndex].fX = vPos.fX + (fRadius * vOffset.fX);
		asVertex[nIndex].fY = vPos.fY + (fRadius * vOffset.fY);
		asVertex[nIndex].fZ = vPos.fZ + (fRadius * vOffset.fZ);

		asVertex[nIndex].fNX = vOffset.fX;
		asVertex[nIndex].fNY = vOffset.fY;
		asVertex[nIndex].fNZ = vOffset.fZ;

		asVertex[nIndex].afColour[0] = (afEndCol[0] * fColScale) + (afStartCol[0] * (1.0f - fColScale));
		asVertex[nIndex].afColour[1] = (afEndCol[1] * fColScale) + (afStartCol[1] * (1.0f - fColScale));
		asVertex[nIndex].afColour[2] = (afEndCol[2] * fColScale) + (afStartCol[2] * (1.0f - fColScale));
	}
}


void ConvertTubeToBezier2 (KnotVertex * asVertex, int nPieces, int nSegments, float fRadius, Vector3 vStart, Vector3 vStartDir, Vector3 vEnd, Vector3 vEndDir, float const * afStartCol, float const * afEndCol) {
	int nPiece;
	int nSegment;
	Vector3 vPos;
	Vector3 vOffset;
	float fStep;
	float fTheta;
	int nIndex;
	Vector3 vNormal;
	Vector3 vIdentity;
	Matrix3 mRotation;
	float fColScale;
	Vector3 vUnit;
	Vector3 vUnitPrevious;
	float fCorrection;
	float fAngle;
	Vector3 vPerpendicular;
	Vector3 vCross;
	Vector3 vSum;
	float fLength;

	fCorrection = 0.0f;
	SetVector3 (vIdentity, 1.0f, 0.0f, 0.0f);
	SetVector3 (vNormal, 1.0f, 0.0f, 0.0f);
	SetVector3 (vPerpendicular, 0.0f, 1.0f, 0.0f);
	SetVector3 (vUnit, 0.0f, 1.0f, 0.0f);
	SetVector3 (vUnitPrevious, 0.0f, 1.0f, 0.0f);
	SetIdentity (& mRotation);

	for (nPiece = 0; nPiece <= nPieces; nPiece++) {
		fColScale = ((float)nPiece / (float)nPieces);

		// Calculate the translation due to the bezier curve 
		fStep = ((float)(nPiece)) / ((float)(nPieces));
		vPos.fX = BEZIER (vStart.fX, vStartDir.fX, vEnd.fX, vEndDir.fX, fStep);
		vPos.fY = BEZIER (vStart.fY, vStartDir.fY, vEnd.fY, vEndDir.fY, fStep);
		vPos.fZ = BEZIER (vStart.fZ, vStartDir.fZ, vEnd.fZ, vEndDir.fZ, fStep);

		// Calculate the rotation due to the curve direction
		vNormal.fX = BEZIERDERIV (vStart.fX, vStartDir.fX, vEnd.fX, vEndDir.fX, fStep);
		vNormal.fY = BEZIERDERIV (vStart.fY, vStartDir.fY, vEnd.fY, vEndDir.fY, fStep);
		vNormal.fZ = BEZIERDERIV (vStart.fZ, vStartDir.fZ, vEnd.fZ, vEndDir.fZ, fStep);

		mRotation = RotationBetweenVectors (& vIdentity, & vNormal);
		vUnitPrevious = vUnit;
		vUnit = MultMatrixVector (& mRotation, & vPerpendicular);

		fAngle = DotProdAngleVector (& vUnitPrevious, & vUnit);

		if (fabs (fAngle) > 0.0001f) {
			vCross = CrossProduct (& vUnitPrevious, & vUnit);
			Normalise (& vCross);
			vSum = AddVectors (& vCross, & vNormal);
			fLength = Length (& vSum);
			if (fLength > 1.0f) {
				fAngle = -fAngle;
			}
		}

		if (fabs (fAngle) > (M_PI / (float)nSegments)) {
			if (fAngle > 0) {
				fCorrection -= fAngle;
			}
			else {
				fCorrection -= fAngle;
			}
		}

/*
		fCorrection -= fAngle;
		fMod = fmodf ((fCorrection), (2.0f * M_PI / (float)nSegments));
		fMod = (2.0f * M_PI / (float)nSegments) - fMod;
		if (fMod > (M_PI / (float)nSegments)) {
			fMod -= (2.0f * M_PI / (float)nSegments);
		}
		fCorrection += fMod;
*/

/*
		fAngle = fmodf (fAngle, (2.0f * M_PI / (float)nSegments));
		if (fAngle > (M_PI / (float)nSegments)) {
			fAngle = fAngle - (2.0f * M_PI / (float)nSegments);
		}

		if (fabs (fAngle) > 0.0001f) {
			vCross = CrossProduct (& vTwist1, & vTwist2);
			Normalise (& vCross);
			vSum = AddVectors (& vCross, & vNormal);
			fLength = Length (& vSum);
			if (fLength > 1.0f) {
				fAngle = -fAngle;
			}
		}
*/






		//mRotation = MultMatrixMatrix (& mRotation, & mRotate);

		for (nSegment = 0; nSegment < nSegments; nSegment++) {
			nIndex = (nPiece * nSegments) + nSegment;
			fTheta = ((float)nSegment * (2.0 * M_PI / (float)nSegments));// + fCorrection;

			vOffset.fX = 0.0f;
			vOffset.fY = sin (fTheta);
			vOffset.fZ = cos (fTheta);

			// Apply the rotation
			vOffset = MultMatrixVector (& mRotation, & vOffset);

			asVertex[nIndex].fX = vPos.fX + (fRadius * vOffset.fX);
			asVertex[nIndex].fY = vPos.fY + (fRadius * vOffset.fY);
			asVertex[nIndex].fZ = vPos.fZ + (fRadius * vOffset.fZ);
			asVertex[nIndex].fNX = vOffset.fX;
			asVertex[nIndex].fNY = vOffset.fY;
			asVertex[nIndex].fNZ = vOffset.fZ;

			asVertex[nIndex].afColour[0] = (afEndCol[0] * fColScale) + (afStartCol[0] * (1.0f - fColScale));
			asVertex[nIndex].afColour[1] = (afEndCol[1] * fColScale) + (afStartCol[1] * (1.0f - fColScale));
			asVertex[nIndex].afColour[2] = (afEndCol[2] * fColScale) + (afStartCol[2] * (1.0f - fColScale));
		}
	}
}

float BezierCalculateLength (Vector3 vStart, Vector3 vStartDir, Vector3 vEnd, Vector3 vEndDir, BezPersist * psBezData) {
	int nPiece;
	Vector3 vPos;
	Vector3 vPosNext;
	Vector3 vDelta;
	float fStep;
	float fLength;

	fLength = 0.0f;
	// Find the start position
	vPos.fX = BEZIER (vStart.fX, vStartDir.fX, vEnd.fX, vEndDir.fX, 0.0f);
	vPos.fY = BEZIER (vStart.fY, vStartDir.fY, vEnd.fY, vEndDir.fY, 0.0f);
	vPos.fZ = BEZIER (vStart.fZ, vStartDir.fZ, vEnd.fZ, vEndDir.fZ, 0.0f);

	for (nPiece = 0; nPiece < psBezData->nPieces; nPiece++) {
		// Calculate the translation due to the bezier curve 
		fStep = ((float)(nPiece + 1)) / ((float)(psBezData->nPieces));
		vPosNext.fX = BEZIER (vStart.fX, vStartDir.fX, vEnd.fX, vEndDir.fX, fStep);
		vPosNext.fY = BEZIER (vStart.fY, vStartDir.fY, vEnd.fY, vEndDir.fY, fStep);
		vPosNext.fZ = BEZIER (vStart.fZ, vStartDir.fZ, vEnd.fZ, vEndDir.fZ, fStep);

		// We're interested in the difference in position between the two
		vDelta = SubtractVectors (& vPos, & vPosNext);

		fLength += Length (& vDelta);

		vPos = vPosNext;
	}

	return fLength;
}

Matrix3 CreateRotationMatrix (Vector3 * pvNormal) {
	Matrix3 mRotation;
	Vector3 vNormal;
	Vector3 vOrth1;
	Vector3 vOrth2;
	int nDim;
	int nDimMin;
	GLfloat fMin;
	
	vNormal = * pvNormal;
	Normalise (& vNormal);

	// Choose the dimension vector that's smallest
	nDimMin = 0;
	fMin = ((GLfloat *)& vNormal)[nDimMin];
	for (nDim = 1; nDim < 3; nDim++) {
		if (absf(((GLfloat *)& vNormal)[nDim]) < fMin) {
			nDimMin = nDim;
			fMin = absf(((GLfloat *)& vNormal)[nDim]);
		}
	}

	vOrth1.fX = 0.0f;
	vOrth1.fY = 0.0f;
	vOrth1.fZ = 0.0f;
	((GLfloat *)& vOrth1)[nDimMin] = 1.0f;
	
	for (nDim = 0; nDim < 3; nDim++) {
		((GLfloat *)& vOrth1)[nDim] -= fMin * ((GLfloat *)& vNormal)[nDim];
	}

	Normalise (& vOrth1);
	vOrth2 = CrossProduct (& vNormal, & vOrth1);
	Normalise (& vOrth2);

	mRotation.fA1 = vNormal.fX;
	mRotation.fA2 = vNormal.fY;
	mRotation.fA3 = vNormal.fZ;

	mRotation.fB1 = vOrth1.fX;
	mRotation.fB2 = vOrth1.fY;
	mRotation.fB3 = vOrth1.fZ;

	mRotation.fC1 = vOrth2.fX;
	mRotation.fC2 = vOrth2.fY;
	mRotation.fC3 = vOrth2.fZ;

	return mRotation;
}

void DisplayBeziers (BezPersist * psBezData) {
	Bezier * psBezier;
	
	glMaterialfv (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, gafDiffuse);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SPECULAR, gafSpecular);
	glMaterialfv (GL_FRONT_AND_BACK, GL_SHININESS, gafShininess);
	glEnable (GL_COLOR_MATERIAL);

	// Render all of the bezier curves
	psBezier = psBezData->psBezierFirst;
	while (psBezier) {
		DrawBezier (0.0, 0.0, 0.0, psBezier, psBezData);
		psBezier = psBezier->psNext;
	}
}

void DrawBezier (GLdouble fXCentre, GLdouble fYCentre, GLdouble fZCentre, Bezier * psBezier, BezPersist * psBezData) {
	int nIndices;
	int nPiece;
	glPushMatrix ();
	glTranslatef (fXCentre, fYCentre, fZCentre);

	glEnableClientState (GL_VERTEX_ARRAY);
	glEnableClientState (GL_NORMAL_ARRAY);
	glEnableClientState (GL_COLOR_ARRAY);
	glBindBuffer (GL_ARRAY_BUFFER, psBezier->auVertexBuffer[0]);

	glVertexPointer (3, GL_FLOAT, sizeof (KnotVertex), 0);
	glNormalPointer (GL_FLOAT, sizeof (KnotVertex), BUFFER_OFFSET (12));
	glColorPointer (BEZ_COL_COMPONENTS, GL_FLOAT, sizeof (KnotVertex), BUFFER_OFFSET (24));

	nIndices = (psBezData->nSegments + 1) * 2;
	for (nPiece = 0; nPiece < psBezData->nPieces; nPiece++) {
		glBindBuffer (GL_ELEMENT_ARRAY_BUFFER, psBezData->auIndexBuffer[nPiece]);
		glDrawRangeElements (GL_TRIANGLE_STRIP, 0, nIndices, nIndices, GL_UNSIGNED_SHORT, BUFFER_OFFSET (0));
	}

	glDisableClientState (GL_VERTEX_ARRAY);
	glDisableClientState (GL_NORMAL_ARRAY);
	glDisableClientState (GL_COLOR_ARRAY);
	glDisableClientState (GL_INDEX_ARRAY);

	//glutSolidCube (0.5);
	glPopMatrix ();
}

// Use De Casteljau's algorithm to split a bezier curve into two subcurves
void SplitBezier (Vector3 vStart, Vector3 * pvStartDir, Vector3 vEnd, Vector3 * pvEndDir, float fRatio, Vector3 * pvMid, Vector3 * pvMidDirBack, Vector3 * pvMidDirForward) {
	Vector3 vMid0;
	Vector3 vMid1;
	Vector3 vMid2;

	if (pvStartDir && pvEndDir) {
		vMid0 = SplitLine (vStart, *pvStartDir, fRatio);
		vMid1 = SplitLine (*pvStartDir, *pvEndDir, fRatio);
		vMid2 = SplitLine (*pvEndDir, vEnd, fRatio);

		if (pvMidDirBack && pvMidDirForward && pvMid) {
			*pvMidDirBack = SplitLine (vMid0, vMid1, fRatio);
			*pvMidDirForward = SplitLine (vMid1, vMid2, fRatio);
			*pvMid = SplitLine (*pvMidDirBack, *pvMidDirForward, fRatio);
		}
		
		*pvStartDir = vMid0;
		*pvEndDir = vMid2;
	}
}

// Find the point along a line between two points based on a proportion of the distance between them
Vector3 SplitLine (Vector3 vStart, Vector3 vEnd, float fRatio) {
	Vector3 vMid;
	
	vMid.fX = (vStart.fX * (1.0f - fRatio)) + (vEnd.fX * (fRatio));
	vMid.fY = (vStart.fY * (1.0f - fRatio)) + (vEnd.fY * (fRatio));
	vMid.fZ = (vStart.fZ * (1.0f - fRatio)) + (vEnd.fZ * (fRatio));
	
	return vMid;
}

void SetAccuracy (int nPieces, int nSegments, BezPersist * psBezData) {
	// Note that in order to prevent serious problems
	// all beziers must be regenerated after changing the accuracy values

	// Free the index buffers
	glDeleteBuffers (psBezData->nPieces, psBezData->auIndexBuffer);
	g_free (psBezData->auIndexBuffer);
	psBezData->auIndexBuffer = NULL;

	// Recreate the index buffers
	psBezData->nPieces = nPieces;
	psBezData->nSegments = nSegments;
	psBezData->auIndexBuffer = CreateBezierIndexBuffers (psBezData->nPieces, psBezData->nSegments);
}

void DeleteBeziers (Bezier * psBezierStart, int nNum, BezPersist * psBezData) {
	Bezier * psBezier;
	Bezier * psBezierNext;
	int nDeleted;

	if (psBezierStart != NULL) {
		psBezier = psBezierStart;
	}
	else {
		psBezier = psBezData->psBezierFirst;
	}

	nDeleted = 0;
	while ((nDeleted < nNum) && (psBezier != NULL)) {
		psBezierNext = psBezier->psNext;
		DeleteBezier (psBezier, psBezData);
		psBezier = psBezierNext;
		nDeleted++;
	}
}

Bezier * GetBezierFirst (BezPersist * psBezData) {
	return psBezData->psBezierFirst;
}

Bezier * GetBezierLast (BezPersist * psBezData) {
	return psBezData->psBezierLast;
}

Bezier * GetBezierNext (Bezier * psBezier) {
	return psBezier->psNext;
}

Bezier * GetBezierPrev (Bezier * psBezier) {
	return psBezier->psPrev;
}


