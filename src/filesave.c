///////////////////////////////////////////////////////////////////
// Functy FileSave.c
// Load/Save Functy function information
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// December 2012
///////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////
// Includes

#include <string.h>
#include <locale.h>

#include "utils.h"
#include "filesave.h"

///////////////////////////////////////////////////////////////////
// Defines

///////////////////////////////////////////////////////////////////
// Structures and enumerations

struct _LocaleRestore {
	GString * szLocale;
};

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

///////////////////////////////////////////////////////////////////
// Function definitions

LocaleRestore * ClearLocale () {
	LocaleRestore * psRestore;
	char * szCurrent;

	szCurrent = setlocale (LC_NUMERIC, NULL);
	psRestore = g_new (LocaleRestore, 1);
	if (szCurrent) {
		psRestore->szLocale = g_string_new (szCurrent);
	}
	else {
		psRestore->szLocale = NULL;
	}

	setlocale (LC_NUMERIC, "C");
	
	return psRestore;
}

void RestoreLocale (LocaleRestore * psRestore) {
	if (psRestore) {
		if (psRestore->szLocale) {
			setlocale (LC_NUMERIC, psRestore->szLocale->str);
			g_string_free (psRestore->szLocale, TRUE);
		}
		else {
			setlocale (LC_NUMERIC, "");
		}
		g_free (psRestore);
	}
}

