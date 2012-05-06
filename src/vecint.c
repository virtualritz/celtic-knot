///////////////////////////////////////////////////////////////////
// VecInt
// Vector Integer structures
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Summer 2011
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include "vecint.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

///////////////////////////////////////////////////////////////////
// Function definitions

VecInt3 AddVecInts (VecInt3 const * pvn1, VecInt3 const * pvn2) {
	VecInt3 vnReturn;

	vnReturn.nX = (pvn1->nX + pvn2->nX);
	vnReturn.nY = (pvn1->nY + pvn2->nY);
	vnReturn.nZ = (pvn1->nZ + pvn2->nZ);

	return vnReturn;
}

int ConvertToIndex (VecInt3 const * pvnPos, VecInt3 const * pvnSize) {
	int nIndex = -1;
	
	if ((pvnPos->nX >= 0) && (pvnPos->nX < pvnSize->nX) 
		&& (pvnPos->nY >= 0) && (pvnPos->nY < pvnSize->nY) 
		&& (pvnPos->nZ >= 0) && (pvnPos->nZ < pvnSize->nZ)) {
		nIndex = (pvnPos->nZ * pvnSize->nX * pvnSize->nY) 
			+ (pvnPos->nY * pvnSize->nX) 
			+ (pvnPos->nX);
	}

	return nIndex;
}


