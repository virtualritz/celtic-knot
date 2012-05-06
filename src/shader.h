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

#ifndef SHADER_H
#define SHADER_H

/* Includes */

#include "utils.h"

/* Defines */

/* Enums */

/* Structures */
typedef struct _ShaderPersist ShaderPersist;

/* Function prototypes */
ShaderPersist * NewShaderPersist ();
void DeleteShaderPersist (ShaderPersist * psShaderData);
void LoadVertexShader (char const * const szFilename, ShaderPersist * psShaderData);
void LoadFragmentShader (char const * const szFilename, ShaderPersist * psShaderData);
void ActivateShader (ShaderPersist * psShaderData);
void DeactivateShader (ShaderPersist * psShaderData);

#endif /* CELTIC_H */

