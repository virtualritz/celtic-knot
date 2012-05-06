/*******************************************************************
 * Celtic2D.c
 * Generate 2D celtic knot patterns
 *
 * David Llewellyn-Jones
 * http://www.flypig.co.uk
 *
 * July 2011
 *******************************************************************
*/

#ifndef CELTIC2D_H
#define CELTIC2D_H

/* Includes */

#include "celtic.h"

/* Defines */

/* Enums */

/* Structures */

/* Function prototypes */
CelticPersist * NewCelticPersist2D (int nWidth, int nHeight, float fTileX, float fTileY, BezPersist * psBezData);

#endif /* CELTIC2D_H */

