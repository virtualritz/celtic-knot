/*******************************************************************
 * Shader.c
 * Manage the OpenGL GLSL shaders
 *
 * David Llewellyn-Jones
 * http://www.flypig.co.uk
 *
 * December 2011
 *******************************************************************
*/

/* Includes */
#include "shader.h"

/* Defines */

/* Enums */

/* Structures */

struct _ShaderPersist {
	GLuint uVertex;
	GLuint uFragment;
	GLuint uProgram;
};

/* Function prototypes */

void LinkShader (GLuint uShader, ShaderPersist * psShaderData);

/* Function defininitions */
ShaderPersist * NewShaderPersist () {
	ShaderPersist * psShaderData;
	GLenum eError;

	psShaderData = (ShaderPersist *)calloc (1, sizeof (ShaderPersist));
	psShaderData->uProgram = glCreateProgram ();
	eError = glGetError ();
	if (eError != 0) {
		fprintf (stderr, "OpenGL program creation failed. Error %d.\n", eError);
	}

	psShaderData->uVertex = 0u;
	psShaderData->uFragment = 0u;

	return psShaderData;
}

void DeleteShaderPersist (ShaderPersist * psShaderData) {
	if (psShaderData) {
		if (psShaderData->uVertex != 0) {
			glDeleteShader (psShaderData->uVertex);
			if (psShaderData->uProgram != 0u) {
				glDetachShader (psShaderData->uProgram, psShaderData->uVertex);
			}
		}

		if (psShaderData->uFragment != 0) {
			glDeleteShader (psShaderData->uFragment);
			if (psShaderData->uProgram != 0u) {
				glDetachShader (psShaderData->uProgram, psShaderData->uFragment);
			}
		}

		if (psShaderData->uProgram != 0u) {
			glDeleteProgram (psShaderData->uProgram);
		}

		free (psShaderData);
		psShaderData = NULL;
	}
}

char const * LoadTextFile (char const * const szFilename) {
	FILE * hFile;
	char * szMemory;
	int nLength;
	int nRead;

	szMemory = NULL;	
	hFile = fopen (szFilename, "r");
	if (hFile) {
		fseek (hFile, 0, SEEK_END);
		nLength = ftell (hFile);
		szMemory = (char *)malloc (nLength + 1);
		szMemory[nLength] = 0;
		fseek (hFile, 0, SEEK_SET);
		nRead = fread (szMemory, 1, nLength, hFile);
		if (nRead < nLength) {
			fprintf (stderr, "Shader file incompletely read.\n");
		}
		fclose (hFile);
	}
	
	return szMemory;
}

void ReleaseTextFile (char const * szMemory) {
	free ((void *)szMemory);
}

void LoadVertexShader (char const * const szFilename, ShaderPersist * psShaderData) {
	char const * szShader;
	GLint nStatus;
	GLint nLogsize;
	char * szLog;
	GLenum eError;

	szShader = LoadTextFile (szFilename);

	if (szShader) {
		psShaderData->uVertex = glCreateShader (GL_VERTEX_SHADER);
		glShaderSource (psShaderData->uVertex, 1, & szShader, NULL);
		glCompileShader (psShaderData->uVertex);
		glGetShaderiv (psShaderData->uVertex, GL_COMPILE_STATUS, & nStatus);
		if (nStatus != GL_TRUE) {
			eError = glGetError ();
			fprintf (stderr, "Vertex shader failed to compile. Error %d.\n", eError);
		}
		glGetShaderiv (psShaderData->uVertex, GL_INFO_LOG_LENGTH, & nLogsize);
		if (nLogsize > 1) {
			szLog = (char *)malloc (nLogsize + 1);
			glGetShaderInfoLog (psShaderData->uVertex, nLogsize + 1, NULL, szLog);
			szLog[nLogsize] = 0;
			printf ("%s\n", szLog);
			free (szLog);
			szLog = NULL;
		}

		ReleaseTextFile (szShader);
		szShader = NULL;

		LinkShader (psShaderData->uVertex, psShaderData);
	}
	else {
		fprintf (stderr, "Vertex shader file could not be read.\n");
	}
}

void LoadFragmentShader (char const * const szFilename, ShaderPersist * psShaderData) {
	char const * szShader;
	GLint nStatus;
	GLint nLogsize;
	char * szLog;
	GLenum eError;

	szShader = LoadTextFile (szFilename);

	if (szShader) {
		psShaderData->uFragment = glCreateShader (GL_FRAGMENT_SHADER);
		glShaderSource (psShaderData->uFragment, 1, & szShader, NULL);
		glCompileShader (psShaderData->uFragment);
		glGetShaderiv (psShaderData->uFragment, GL_COMPILE_STATUS, & nStatus);
		if (nStatus != GL_TRUE) {
			eError = glGetError ();
			fprintf (stderr, "Fragment shader failed to compile. Error %d.\n", eError);
		}
		glGetShaderiv (psShaderData->uFragment, GL_INFO_LOG_LENGTH, & nLogsize);
		if (nLogsize > 1) {
			szLog = (char *)malloc (nLogsize + 1);
			glGetShaderInfoLog (psShaderData->uFragment, nLogsize + 1, NULL, szLog);
			szLog[nLogsize] = 0;
			printf ("%s\n", szLog);
			free (szLog);
			szLog = NULL;
		}

		ReleaseTextFile (szShader);
		szShader = NULL;

		LinkShader (psShaderData->uFragment, psShaderData);
	}
	else {
		fprintf (stderr, "Fragment shader file could not be read.\n");
	}
}

void LinkShader (GLuint uShader, ShaderPersist * psShaderData) {
	GLint nStatus;
	GLint nLogsize;
	char * szLog;
	GLenum eError;

	glAttachShader (psShaderData->uProgram, uShader);
	
	glLinkProgram (psShaderData->uProgram);
	glGetProgramiv (psShaderData->uProgram, GL_LINK_STATUS, & nStatus);
	if (nStatus != GL_TRUE) {
		eError = glGetError ();
		fprintf (stderr, "Shader failed to link. Error %d.\n", eError);
	}
	glGetProgramiv (psShaderData->uProgram, GL_INFO_LOG_LENGTH, & nLogsize);
	if (nLogsize > 1) {
		szLog = (char *)malloc (nLogsize + 1);
		glGetProgramInfoLog (psShaderData->uProgram, nLogsize + 1, NULL, szLog);
		szLog[nLogsize] = 0;
		printf ("%s\n", szLog);
		free (szLog);
		szLog = NULL;
	}
}

void ActivateShader (ShaderPersist * psShaderData) {
	if (psShaderData->uProgram != 0u) {
		if ((psShaderData->uVertex != 0) || (psShaderData->uFragment != 0)) {
			glUseProgram (psShaderData->uProgram);
		}
	}
}

void DeactivateShader (ShaderPersist * psShaderData) {
	glUseProgram (0u);
}

