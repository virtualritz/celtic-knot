///////////////////////////////////////////////////////////////////
// Utils
// Generally useful definitions, structures, functions, etc.
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2008
///////////////////////////////////////////////////////////////////

#ifndef UTILS_H
#define UTILS_H

///////////////////////////////////////////////////////////////////
// Includes

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include <GLee.h>
#include <glib.h>
#include <GL/glut.h>
#include <gtk/gtk.h>

///////////////////////////////////////////////////////////////////
// Defines

#ifndef _WIN32
#define stricmp strcasecmp
#endif

#define BEZIER(P1, D1, P2, D2, STEP) ((pow ((1.0 - (STEP)), 3.0) * (double)(P1)) + (3.0 * pow ((1.0 - (STEP)), 2.0) * (STEP) * (double)(D1)) + (3.0 * (1.0 - (STEP)) * pow ((STEP), 2.0) * (double)(D2)) + (pow ((STEP), 3.0) * (P2)));

#define BEZIERDERIV(P1, D1, P2, D2, STEP) ((3.0 * pow ((1.0 - (STEP)), 2.0) * ((double)(D1) - (double)(P1))) + (6.0 * (1.0 - (STEP)) * (STEP) * ((double)(D2) - (double)(D1))) + (3.0 * pow ((STEP), 2.0) * ((double)(P2) - (double)(D2))))

#define SetVector3(SET, X, Y, Z) (SET).fX = (X); (SET).fY = (Y); (SET).fZ = (Z);

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef int bool;

typedef struct _Vector3 {
	GLfloat fX;
	GLfloat fY;
	GLfloat fZ;
} Vector3;

// [ a1 b1 c1 ]
// [ a2 b2 c2 ]
// [ a3 b3 c3 ]
typedef struct _Matrix3 {
	GLfloat fA1;
	GLfloat fA2;
	GLfloat fA3;
	GLfloat fB1;
	GLfloat fB2;
	GLfloat fB3;
	GLfloat fC1;
	GLfloat fC2;
	GLfloat fC3;
} Matrix3;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

float absf (float fValue);
Vector3 Normal (Vector3 * v1, Vector3 * v2);
Vector3 AddVectors (Vector3 const * v1, Vector3 const * v2);
Vector3 SubtractVectors (Vector3 const * v1, Vector3 const * v2);
Vector3 MultiplyVectors (Vector3 const * v1, Vector3 const * v2);
Vector3 ScaleVector (Vector3 const * v1, float fScale);
void Normalise (Vector3 * v1);
void Normalise3f (float * pfX, float * pfY, float * pfZ);
float Length (Vector3 * v1);
Matrix3 Invert (Matrix3 * m1);
float Determinant (Matrix3 * m1);
float DotProdAngle (float fX1, float fY1, float fX2, float fY2);
float DotProdAngleVector (Vector3 * v1, Vector3 * v2);
Vector3 MultMatrixVector (Matrix3 * m1, Vector3 * v1);
void PrintMatrix (Matrix3 * m1);
void PrintVector (Vector3 * v1);
Vector3 CrossProduct (Vector3 * v1, Vector3 * v2);
Matrix3 MultMatrixMatrix (Matrix3 * m1, Matrix3 * m2);
void SetIdentity (Matrix3 * m1);
Matrix3 RotationBetweenVectors (Vector3 * v1, Vector3 * v2);
Matrix3 RotationAngleAxis (Vector3 * vAxis, float fAngle);
Vector3 PerpendicularVector (Vector3 * pvVector);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* UTILS_H */

