///////////////////////////////////////////////////////////////////
// Bezier
// Render bezier tubes
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Summber 2011
///////////////////////////////////////////////////////////////////

#ifndef BEZ_H
#define BEZ_H

///////////////////////////////////////////////////////////////////
// Includes

#include "utils.h"

///////////////////////////////////////////////////////////////////
// Defines

#define BEZ_COL_COMPONENTS (3)

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _BezPersist BezPersist;
typedef struct _Bezier Bezier;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

BezPersist * NewBezPersist (int nPieces, int nSegments);
void DeleteBezPersist (BezPersist * psBezData);
void DisplayBeziers (BezPersist * psBezData);
Bezier * NewBezier (BezPersist * psBezData);
void DeleteBezier (Bezier * psBezier, BezPersist * psBezData);
void DeleteBeziers (Bezier * psBezierStart, int nNum, BezPersist * psBezData);
void SetBezierControlPoints (float fRadius, Vector3 vStart, Vector3 vStartDir, Vector3 vEnd, Vector3 vEndDir, float const * afStartCol, float const * afEndCol, Bezier * psBezier, BezPersist * psBezData);
void SplitBezier (Vector3 vStart, Vector3 * pvStartDir, Vector3 vEnd, Vector3 * pvEndDir, float fRatio, Vector3 * pvMid, Vector3 * pvMidDirBack, Vector3 * pvMidDirForward);
Vector3 SplitLine (Vector3 vStart, Vector3 vEnd, float fRatio);
Bezier * GetBezierFirst (BezPersist * psBezData);
Bezier * GetBezierLast (BezPersist * psBezData);
Bezier * GetBezierNext (Bezier * psBezier);
Bezier * GetBezierPrev (Bezier * psBezier);
void StoreBeziers (bool boStore, BezPersist * psBezData);
bool OutputStoredBeziers (char const * szFilename, BezPersist * psBezData, bool boBinary);
float BezierCalculateLength (Vector3 vStart, Vector3 vStartDir, Vector3 vEnd, Vector3 vEndDir, BezPersist * psBezData);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* BEZ_H */


