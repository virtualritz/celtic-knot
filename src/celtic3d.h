/*******************************************************************
 * Celtic3D.h
 * Generate 3D celtic knot patterns
 *
 * David Llewellyn-Jones
 * http://www.flypig.co.uk
 *
 * July 2011
 *******************************************************************
*/

#ifndef CELTIC3D_H
#define CELTIC3D_H

/* Includes */

#include "celtic.h"

/* Defines */

/* Enums */

/* Structures */

/* Function prototypes */
CelticPersist * NewCelticPersist3D (int nWidth, int nHeight, int nDepth, float fTileX, float fTileY, float fTileZ, BezPersist * psBezData);

#endif /* CELTIC3D_H */

