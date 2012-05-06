#version 120

#define RED 0
#define GREEN 1
#define BLUE 2

varying vec4 vDiffuse, vGlobal, vAmbient;
varying vec3 vNormal, vDir, vHalfVector;
varying float fDist;
const float fEpsilon = 1e-6;

#define swap(A,B) {int C = A; A = B; B = C;}

int mod (int nDividend, int nDivisor) {
	int nRemainder;
	int nMultiple;
	
	nMultiple = nDividend / nDivisor;
	
	nRemainder = nDividend - (nMultiple * nDivisor);

	return nRemainder;
}

// Note: hue, saturation and range values all fall within the interval 0 to 1.
// Usually this is 0...360, 0...100 and 0...256 respectively.
vec3 RGBtoHSV (vec3 vColour) {
	float fHue;
	float fSaturation;
	float fValue;
	float fDiff;
	float fMinCol;
	float fMaxCol;
	int nMinIndex;
	int nMaxIndex;

	nMinIndex = (vColour.r < vColour.g ? (vColour.b < vColour.r ? BLUE : RED) : (vColour.b < vColour.g ? BLUE : GREEN));
	nMaxIndex = (vColour.r > vColour.g ? (vColour.b > vColour.r ? BLUE : RED) : (vColour.b > vColour.g ? BLUE : GREEN));
	fMinCol = vColour[nMinIndex];
	fMaxCol = vColour[nMaxIndex];
	fDiff = fMaxCol - fMinCol;

	// Calculate Hue
	if (fDiff < fEpsilon){
		fHue = 0.0;
	}
	else if (nMaxIndex == RED) {
		fHue = ((1.0 / 6.0) * ((vColour.g - vColour.b) / fDiff)) + 1.0;
		fHue = fract (fHue);
	}
	else if (nMaxIndex == GREEN) {
		fHue = ((1.0 / 6.0) * ((vColour.b - vColour.r) / fDiff)) + (1.0 / 3.0);
	}
	else if (nMaxIndex == BLUE) {
		fHue = ((1.0 / 6.0) * ((vColour.r - vColour.g) / fDiff)) + (2.0 / 3.0);        
	}

	// Saturation
	if (fMaxCol < fEpsilon) {
		fSaturation = 0.0;
	}
	else {
		fSaturation = fDiff / fMaxCol;
	}

	// Value
	fValue = fMaxCol;

	return vec3 (fHue, fSaturation, fValue);
}

vec3 HSVtoRGB (vec3 vColour) {
	float fF;
	float fP;
	float fQ;
	float fT;
	int nSector;
	float fSector;
	float fHue;
	float fSaturation;
	float fValue;
	vec3 vResult;

	fHue = vColour.r;
	fSaturation = vColour.g;
	fValue = vColour.b;

	fSector = floor (fHue * 6.0);
	nSector = mod (int (fSector), 6);
	fF = (fHue * 6.0) - fSector;
	fP = fValue * (1.0 - fSaturation);
	fQ = fValue * (1.0 - (fF * fSaturation));
	fT = fValue * (1.0 - ((1.0 - fF) * fSaturation));

	if (nSector == 0) {
		vResult = vec3 (fValue, fT, fP);
	}
	else if (nSector == 1) {
		vResult = vec3 (fQ, fValue, fP);
	}
	else if (nSector == 2) {
		vResult = vec3 (fP, fValue, fT);
	}
	else if (nSector == 3) {
		vResult = vec3 (fP, fQ, fValue);
	}
	else if (nSector == 4) {
		vResult = vec3 (fT, fP, fValue);
	}
	else {
		vResult = vec3 (fValue, fP, fQ);
	}

	return vResult;
}

void main () {
	vec3 vHalf;
	vec3 vView;
	vec3 dir;
	float fNDotL;
	float fNDotHV;
	vec4 vColour = vGlobal;
	float fAttenuation;
	float fHueSteps = 88.0;
	float fSteps = 2.0;
	vec3 vRGB;
	vec3 vHSV;

	// Calculate dot product between normal and light direction
	fNDotL = max (dot (vNormal, normalize (vDir)), 0.0);

	if (false) //(fNDotL > 0.0) {
		fAttenuation = 1.0 / (gl_LightSource[0].constantAttenuation + gl_LightSource[0].linearAttenuation * fDist + gl_LightSource[0].quadraticAttenuation * fDist * fDist);
		vColour += fAttenuation * (vDiffuse * fNDotL + vAmbient);

		vHalf = normalize (vHalfVector);
		fNDotHV = max (dot (vNormal, vHalf), 0.0);
		vColour += fAttenuation * gl_FrontMaterial.specular * gl_LightSource[0].specular * pow (fNDotHV, gl_FrontMaterial.shininess);
		vRGB = vColour.rgb;
		vHSV = RGBtoHSV (vRGB);
		vHSV[0] = floor (vHSV[0] * fHueSteps) / fHueSteps;
		vHSV[1] = floor (vHSV[1] * fSteps) / fSteps;
		vHSV[2] = floor (vHSV[2] * fSteps) / fSteps;
		vRGB = HSVtoRGB (vHSV);
		vColour.rgb = vRGB;
	}

	gl_FragColor = vColour;
}

