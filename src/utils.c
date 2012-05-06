///////////////////////////////////////////////////////////////////
// Utils
// Generally useful definitions, structures, functions, etc.
//
// David Llewellyn-Jones
// Liverpool John Moores University
//
// Spring 2008
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include "utils.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

///////////////////////////////////////////////////////////////////
// Global variables

static Matrix3 mId0 = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

///////////////////////////////////////////////////////////////////
// Function prototypes

///////////////////////////////////////////////////////////////////
// Function definitions

float absf (float fValue) {
	if (fValue < 0.0f) fValue = -fValue;
	return fValue;
}

Vector3 Normal (Vector3 * v1, Vector3 * v2) {
	Vector3 vReturn;

	vReturn.fX = (v1->fY * v2->fZ) - (v1->fZ * v2->fY);
	vReturn.fY = (v1->fZ * v2->fX) - (v1->fX * v2->fZ);
	vReturn.fZ = (v1->fX * v2->fY) - (v1->fY * v2->fX);

	Normalise (& vReturn);

	return vReturn;
}

Vector3 AddVectors (Vector3 const * v1, Vector3 const * v2) {
	Vector3 vReturn;

	vReturn.fX = (v1->fX + v2->fX);
	vReturn.fY = (v1->fY + v2->fY);
	vReturn.fZ = (v1->fZ + v2->fZ);

	return vReturn;
}

Vector3 SubtractVectors (Vector3 const * v1, Vector3 const * v2) {
	Vector3 vReturn;

	vReturn.fX = (v1->fX - v2->fX);
	vReturn.fY = (v1->fY - v2->fY);
	vReturn.fZ = (v1->fZ - v2->fZ);

	return vReturn;
}

Vector3 MultiplyVectors (Vector3 const * v1, Vector3 const * v2) {
	Vector3 vReturn;

	vReturn.fX = (v1->fX * v2->fX);
	vReturn.fY = (v1->fY * v2->fY);
	vReturn.fZ = (v1->fZ * v2->fZ);

	return vReturn;
}

Vector3 ScaleVector (Vector3 const * v1, float fScale) {
	Vector3 vReturn;

	vReturn.fX = (v1->fX * fScale);
	vReturn.fY = (v1->fY * fScale);
	vReturn.fZ = (v1->fZ * fScale);

	return vReturn;
}

void Normalise (Vector3 * v1) {
	float fLength;

	fLength = sqrt ((v1->fX * v1->fX) + (v1->fY * v1->fY) + (v1->fZ * v1->fZ));

	v1->fX /= fLength;
	v1->fY /= fLength;
	v1->fZ /= fLength;
}

void Normalise3f (float * pfX, float * pfY, float * pfZ) {
	float fLength;

	fLength = sqrt (((*pfX) * (*pfX)) + ((*pfY) * (*pfY)) + ((*pfZ) * (*pfZ)));

	*pfX /= fLength;
	*pfY /= fLength;
	*pfZ /= fLength;
}

float Length (Vector3 * v1) {
	return sqrt ((v1->fX * v1->fX) + (v1->fY * v1->fY) + (v1->fZ * v1->fZ));
}

Matrix3 Invert (Matrix3 * m1) {
	Matrix3 vReturn;
	float fDet;

	fDet = Determinant (m1);
	if (fDet != 0.0f) {
		fDet = 1 / fDet;

		vReturn.fA1 =   fDet * ((m1->fB2 * m1->fC3) - (m1->fC2 * m1->fB3));
		vReturn.fA2 = - fDet * ((m1->fA2 * m1->fC3) - (m1->fC2 * m1->fA3));
		vReturn.fA3 =   fDet * ((m1->fA2 * m1->fB3) - (m1->fB2 * m1->fA3));

		vReturn.fB1 = - fDet * ((m1->fB1 * m1->fC3) - (m1->fC1 * m1->fB3));
		vReturn.fB2 =   fDet * ((m1->fA1 * m1->fC3) - (m1->fC1 * m1->fA3));
		vReturn.fB3 = - fDet * ((m1->fA1 * m1->fB3) - (m1->fB1 * m1->fA3));

		vReturn.fC1 =   fDet * ((m1->fB1 * m1->fC2) - (m1->fC1 * m1->fB2));
		vReturn.fC2 = - fDet * ((m1->fA1 * m1->fC2) - (m1->fC1 * m1->fA2));
		vReturn.fC3 =   fDet * ((m1->fA1 * m1->fB2) - (m1->fB1 * m1->fA2));
	}
	else {
		vReturn = mId0;
	}

	return vReturn;
}

float Determinant (Matrix3 * m1) {
	return (m1->fA1 * ((m1->fB2 * m1->fC3) - (m1->fB3 * m1->fC2)))
		- (m1->fA2 * ((m1->fB1 * m1->fC3) - (m1->fB3 * m1->fC1)))
		+ (m1->fA3 * ((m1->fB1 * m1->fC2) - (m1->fB2 * m1->fC1)));
}

float DotProdAngle (float fX1, float fY1, float fX2, float fY2) {
	float fAngle;
	float fScaler;

	float fY;
	float fRot;

	fRot = atan2 (fY1, fX1);
	fY = - (fX2 * sin (fRot)) + (fY2 * cos (fRot));

	fScaler = sqrt ((fX1 * fX1) + (fY1 * fY1)) * sqrt ((fX2 * fX2) + (fY2 * fY2));
	fAngle = acos (((fX1 * fX2) + (fY1 * fY2)) / fScaler);

	if (fY < 0) fAngle = -fAngle;

	return fAngle;
}

float DotProdAngleVector (Vector3 * v1, Vector3 * v2) {
	float fAngle;
	float fScaler;
	float fDotProduct;
	float fQuotient;

	fScaler = Length (v1) * Length (v2);
	fDotProduct = ((v1->fX * v2->fX) + (v1->fY * v2->fY) + (v1->fZ * v2->fZ));
	fQuotient = (fDotProduct / fScaler);
	fQuotient = CLAMP (fQuotient, -1.0f, 1.0f);
	fAngle = acos (fQuotient);

	return fAngle;
}

// Useful for calculating the result of simultaneous equations
// and therefore where the normal to a plane passes through a point
// [ a1 b1 c1 ] [ v1 ]   [ r1 ]   [ (a1*v1) + (b1*v2) + (c1*v3) ]
// [ a2 b2 c2 ] [ v2 ] = [ r3 ] = [ (a2*v1) + (b2*v2) + (c2*v3) ]
// [ a3 b3 c3 ] [ v3 ]   [ r3 ]   [ (a3*v1) + (b3*v2) + (c3*v3) ]
Vector3 MultMatrixVector (Matrix3 * m1, Vector3 * v1) {
	Vector3 vReturn;
	vReturn.fX = (m1->fA1 * v1->fX) + (m1->fB1 * v1->fY) + (m1->fC1 * v1->fZ);
	vReturn.fY = (m1->fA2 * v1->fX) + (m1->fB2 * v1->fY) + (m1->fC2 * v1->fZ);
	vReturn.fZ = (m1->fA3 * v1->fX) + (m1->fB3 * v1->fY) + (m1->fC3 * v1->fZ);

	return vReturn;
}

void PrintMatrix (Matrix3 * m1) {
	printf ("[ %f, \t%f, \t%f \t]\n", m1->fA1, m1->fB1, m1->fC1);
	printf ("[ %f, \t%f, \t%f \t]\n", m1->fA2, m1->fB2, m1->fC2);
	printf ("[ %f, \t%f, \t%f \t]\n", m1->fA3, m1->fB3, m1->fC3);
}

void PrintVector (Vector3 * v1) {
	printf ("[ %f, \t%f, \t%f \t]\n", v1->fX, v1->fY, v1->fZ);
}

Vector3 CrossProduct (Vector3 * v1, Vector3 * v2) {
	Vector3 vResult;
	
	vResult.fX = (v1->fY * v2->fZ) - (v1->fZ * v2->fY);
	vResult.fY = (v1->fZ * v2->fX) - (v1->fX * v2->fZ);
	vResult.fZ = (v1->fX * v2->fY) - (v1->fY * v2->fX);

	return vResult;
}

Matrix3 MultMatrixMatrix (Matrix3 * m1, Matrix3 * m2) {
	Matrix3 mResult;
	
	mResult.fA1 = (m1->fA1 * m2->fA1) + (m1->fA2 * m2->fB1) + (m1->fA3 * m2->fC1);
	mResult.fA2 = (m1->fA1 * m2->fA2) + (m1->fA2 * m2->fB2) + (m1->fA3 * m2->fC2);
	mResult.fA3 = (m1->fA1 * m2->fA3) + (m1->fA2 * m2->fB3) + (m1->fA3 * m2->fC3);

	mResult.fB1 = (m1->fB1 * m2->fA1) + (m1->fB2 * m2->fB1) + (m1->fB3 * m2->fC1);
	mResult.fB2 = (m1->fB1 * m2->fA2) + (m1->fB2 * m2->fB2) + (m1->fB3 * m2->fC2);
	mResult.fB3 = (m1->fB1 * m2->fA3) + (m1->fB2 * m2->fB3) + (m1->fB3 * m2->fC3);

	mResult.fC1 = (m1->fC1 * m2->fA1) + (m1->fC2 * m2->fB1) + (m1->fC3 * m2->fC1);
	mResult.fC2 = (m1->fC1 * m2->fA2) + (m1->fC2 * m2->fB2) + (m1->fC3 * m2->fC2);
	mResult.fC3 = (m1->fC1 * m2->fA3) + (m1->fC2 * m2->fB3) + (m1->fC3 * m2->fC3);

	return mResult;
}

void SetIdentity (Matrix3 * m1) {
	m1->fA1 = 1.0f;
	m1->fA2 = 0.0f;
	m1->fA3 = 0.0f;
	m1->fB1 = 0.0f;
	m1->fB2 = 1.0f;
	m1->fB3 = 0.0f;
	m1->fC1 = 0.0f;
	m1->fC2 = 0.0f;
	m1->fC3 = 1.0f;
}

Matrix3 RotationBetweenVectors (Vector3 * v1, Vector3 * v2) {
	Matrix3 mReturn;
	float fAngle;
	Vector3 vAxis;

	Normalise (v1);
	Normalise (v2);
	fAngle = DotProdAngleVector (v1, v2);
	if (fAngle == 0.0f) {
		SetIdentity (& mReturn);
	}
	else {
		if (fAngle >= M_PI) {
			vAxis = PerpendicularVector (v1);
			mReturn = RotationAngleAxis (& vAxis, fAngle);
		}
		else {
			vAxis = CrossProduct (v1, v2);
			Normalise (& vAxis);
			mReturn = RotationAngleAxis (& vAxis, fAngle);
		}
	}

	return mReturn;
}

Matrix3 RotationAngleAxis (Vector3 * vAxis, float fAngle) {
	Matrix3 mReturn;
	
	mReturn.fA1 = 1.0 + (1.0 - cos (fAngle)) * (vAxis->fX * vAxis->fX - 1.0);
	mReturn.fB1 = -vAxis->fZ * sin (fAngle) + (1.0 - cos (fAngle)) * vAxis->fX * vAxis->fY;
	mReturn.fC1 = vAxis->fY * sin (fAngle) + (1.0 - cos (fAngle)) * vAxis->fX * vAxis->fZ;


	mReturn.fA2 = vAxis->fZ * sin (fAngle) + (1.0 - cos (fAngle)) * vAxis->fX * vAxis->fY;
	mReturn.fB2 = 1.0 + (1.0 - cos (fAngle)) * (vAxis->fY * vAxis->fY - 1.0);
	mReturn.fC2 = -vAxis->fX * sin (fAngle) + (1.0 - cos (fAngle)) * vAxis->fY * vAxis->fZ;


	mReturn.fA3 = -vAxis->fY * sin (fAngle) + (1.0 - cos (fAngle)) * vAxis->fX * vAxis->fZ;
	mReturn.fB3 = vAxis->fX * sin (fAngle) + (1.0 - cos (fAngle)) * vAxis->fY * vAxis->fZ;
	mReturn.fC3 = 1.0 + (1.0 - cos (fAngle)) * (vAxis->fZ * vAxis->fZ - 1.0);

	return mReturn;
}

Vector3 PerpendicularVector (Vector3 * pvVector) {
	Vector3 vVector;
	Vector3 vOrth;
	int nDim;
	int nDimMin;
	GLfloat fMin;
	
	vVector = * pvVector;
	Normalise (& vVector);

	// Choose the dimension vector that's smallest
	nDimMin = 0;
	fMin = ((GLfloat *)& vVector)[nDimMin];
	for (nDim = 1; nDim < 3; nDim++) {
		if (absf(((GLfloat *)& vVector)[nDim]) < fMin) {
			nDimMin = nDim;
			fMin = absf(((GLfloat *)& vVector)[nDim]);
		}
	}

	fMin = ((GLfloat *)& vVector)[nDimMin];
	vOrth.fX = 0.0f;
	vOrth.fY = 0.0f;
	vOrth.fZ = 0.0f;
	((GLfloat *)& vOrth)[nDimMin] = 1.0f;
	
	for (nDim = 0; nDim < 3; nDim++) {
		((GLfloat *)& vOrth)[nDim] -= fMin * ((GLfloat *)& vVector)[nDim];
	}

	Normalise (& vOrth);

	return vOrth;
}



