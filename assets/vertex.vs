#version 120

varying vec4 vDiffuse, vGlobal, vAmbient;
varying vec3 vNormal, vDir, vHalfVector;
varying float fDist;

void main() {	
	vec4 vEyeCoordsPos;
	vec3 vPos;

	// Transform normal to eye space
	vNormal = normalize (gl_NormalMatrix * gl_Normal);

	// Normalise light direction, which is stored in eye space
	vEyeCoordsPos = gl_ModelViewMatrix * gl_Vertex;
	vPos = vec3 (gl_LightSource[0].position - vEyeCoordsPos);
	vDir = normalize (vPos);

	// Calculate distance to light source
	fDist = length (vPos);

	// Normalise half vector; this gets sent to the fragment shader
	vHalfVector = normalize (gl_LightSource[0].halfVector.xyz);

	// Calculate diffuse, ambient and global values
	//vDiffuse = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse * gl_Color;
	//vAmbient = gl_FrontMaterial.ambient * gl_LightSource[0].ambient * gl_Color;
	//vGlobal = gl_LightModel.ambient * gl_FrontMaterial.ambient * gl_Color;
	vDiffuse = gl_LightSource[0].diffuse * gl_Color;
	vAmbient = gl_LightSource[0].ambient * gl_Color;
	vGlobal = gl_LightModel.ambient * gl_Color;
	
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;	
}

