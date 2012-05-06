///////////////////////////////////////////////////////////////////
// VecInt
// Vector Integer structures
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Summer 2011
///////////////////////////////////////////////////////////////////

#ifndef VECINT_H
#define VECINT_H

///////////////////////////////////////////////////////////////////
// Includes

#define _USE_MATH_DEFINES
#include <math.h>

#include <glib.h>
#include <GL/glut.h>
#include <gtk/gtk.h>

///////////////////////////////////////////////////////////////////
// Defines

#define SetVecInt3(SET, X, Y, Z) (SET).nX = (X); (SET).nY = (Y); (SET).nZ = (Z);

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _VecInt3 {
	int nX;
	int nY;
	int nZ;
} VecInt3;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

VecInt3 AddVecInts (VecInt3 const * pvn1, VecInt3 const * pvn2);
int ConvertToIndex (VecInt3 const * pvnPos, VecInt3 const * pvnSize);

///////////////////////////////////////////////////////////////////
// Function definitions

#endif /* VECINT_H */

