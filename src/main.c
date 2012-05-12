///////////////////////////////////////////////////////////////////
// Viewer application
// Display 3D things in a window
//
// David Llewellyn-Jones
// http://www.flypig.co.uk
//
// Winter 2010/2011
//
// Based on GtkGLExt logo demo
// by Naofumi Yasufuku <naofumi@users.sourceforge.net>
///////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////
// Includes

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <glib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <gtk/gtkgl.h>
#include <glade/glade.h>

#include "vis.h"
#include "settings.h"
#include "celtic.h"

///////////////////////////////////////////////////////////////////
// Defines

#define UPDATE_TIMEOUT (10)
#define DEFAULT_PORT (4972)
#define FULLSCREEN_BUTTONBARSHOW (8)
#define RADIUS_SCROLL (1)
#define DEFAULT_FILENAME "Knot.xml"
#define DEFAULT_SETTINGS_FILE ".knot.xml"
#define DEFAULT_EXPORTMODELNAME "Model.ply"
#define DEFAULT_EXPORTBITMAPNAME "Bitmap.png"

///////////////////////////////////////////////////////////////////
// Structures and enumerations

typedef struct _MainPersist MainPersist;

struct _MainPersist {
	VisPersist * psVisData;
	GtkWidget * psDrawingArea;
	gboolean boTimeoutContinue;
	guint TimeoutID;
	GladeXML * psXML;
	gboolean boButtonBarHidden;
	bool boPaused;
  bool boFileLoaded;
  GString * szFilename;
  bool boFolderSet;
  GString * szFolder;
  bool boExportedModel;
  GString * szExportModelName;
  bool boBinary;
  bool boExportedBitmap;
  GString * szExportBitmapName;
  bool boBitmapScreenDimensions;
  int nBitmapWidth;
  int nBitmapHeight;
};

typedef struct _BitmapSize {
  GtkWidget * psWindowSize;
  GtkWidget * psWidth;
  GtkWidget * psHeight;
  GtkWidget * psWidthLabel;
  GtkWidget * psHeightLabel;
} BitmapSize;

///////////////////////////////////////////////////////////////////
// Global variables

///////////////////////////////////////////////////////////////////
// Function prototypes

MainPersist * NewMainPersist (void);
void DeleteMainPersist (MainPersist * psMainData);
static void Realize (GtkWidget * psWidget, gpointer psData);
static void Unrealize (GtkWidget * psWidget, gpointer psData) ;
static gboolean ConfigureEvent (GtkWidget * psWidget, GdkEventConfigure * psEvent, gpointer psData);
static gboolean ExposeEvent (GtkWidget * psWidget, GdkEventExpose * psEvent, gpointer psData);
static gboolean ButtonPressEvent (GtkWidget * psWidget, GdkEventButton * psEvent, gpointer * psData);
static gboolean ButtonReleaseEvent (GtkWidget * psWidget, GdkEventButton * psEvent, gpointer * psData);
static gboolean MotionNotifyEvent (GtkWidget * psWidget, GdkEventMotion * psEvent, gpointer psData);
static gboolean ScrollEvent (GtkWidget * psWidget, GdkEventScroll * psEvent, gpointer psData);
static gboolean KeyPressEvent (GtkWidget * psWidget, GdkEventKey * psEvent, gpointer psData);
static gboolean KeyReleaseEvent (GtkWidget * psWidget, GdkEventKey * psEvent, gpointer psData);
static gboolean Timeout (gpointer psData);
static void TimeoutAdd (MainPersist * psMainData);
static void TimeoutRemove (MainPersist * psMainData);
static gboolean MapEvent (GtkWidget * psWidget, GdkEventAny * psEvent, gpointer psData);
static gboolean UnmapEvent (GtkWidget * psWidget, GdkEventAny * psEvent, gpointer psData);
static gboolean VisibilityNotifyEvent (GtkWidget * psWidget, GdkEventVisibility * psEvent, gpointer psData);
//static gboolean InvertBackground (GtkWidget * psWidget, gpointer psData);
static gboolean SpinVisualisation (GtkWidget * psWidget, gpointer psData);
static gboolean ConfigureDialogueOpen (GtkWidget * psWidget, gpointer psData);
static gboolean ConfigureDialogueOK (GtkWidget * psWidget, gpointer psData);
static gboolean ConfigureDialogueApply (GtkWidget * psWidget, gpointer psData);
void SetConfigureDialogue (MainPersist * psMainData);
void SetConfigureValues (MainPersist * psMainData);
void ToggleFullScreenWindow (MainPersist * psMainData);
void SetDisplayPropertiesDialogue (MainPersist * psMainData);
void SetDisplayPropertiesValues (MainPersist * psMainData);
static gboolean DisplayPropertiesSpinChanged (GtkWidget * psWidget, gpointer psData);
static gboolean DisplayPropertiesSliderChanged (GtkWidget * psWidget, gpointer psData);
void SaveSettingsAll (MainPersist * psMainData);
void LoadSettingsAll (MainPersist * psMainData);
void MainLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData);
void MainLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData);
void MainLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData);
static gboolean ConfigureGenerateSeed (GtkWidget * psWidget, gpointer psData);
static gboolean ConfigureGenerateColourSeed (GtkWidget * psWidget, gpointer psData);
bool LoadFile (char const * szFilename, MainPersist * psMainData);
bool SaveFile (char const * szFilename, MainPersist * psMainData);
bool ExportModelFile (char const * szFilename, MainPersist * psMainData);
bool ExportBitmapFile (char const * szFilename, int nHeight, int nWidth, MainPersist * psMainData);
gchar * GetTitleFilename (char const * szFilename);
void SetMainWindowTitle (char const * szFilename, MainPersist * psMainData);
static gboolean LoadFilePress (GtkWidget * psWidget, gpointer psData);
static gboolean SaveFilePress (GtkWidget * psWidget, gpointer psData);
static gboolean ExportModelPress (GtkWidget * psWidget, gpointer psData);
static gboolean ExportBitmapPress (GtkWidget * psWidget, gpointer psData);
void PauseAnimation (bool boPause, MainPersist * psMainData);
static gboolean BitmapWindowSizeToggle (GtkWidget * psWidget, gpointer psData);

///////////////////////////////////////////////////////////////////
// Function definitions

MainPersist * NewMainPersist (void) {
	MainPersist * psMainData;

	psMainData = g_new0 (MainPersist, 1);

	psMainData->psVisData = NULL;
	psMainData->psDrawingArea = NULL;
	psMainData->boTimeoutContinue = FALSE;
	psMainData->TimeoutID = 0;
	psMainData->boButtonBarHidden = FALSE;
	psMainData->boPaused = FALSE;
  psMainData->boFileLoaded = FALSE;
  psMainData->szFilename = g_string_new (DEFAULT_FILENAME);
  psMainData->boFolderSet = FALSE;
  psMainData->szFolder = g_string_new ("");
  psMainData->boExportedModel = FALSE;
  psMainData->szExportModelName = g_string_new (DEFAULT_EXPORTMODELNAME);
  psMainData->boBinary = TRUE;
  psMainData->boExportedBitmap = FALSE;
  psMainData->szExportBitmapName = g_string_new (DEFAULT_EXPORTBITMAPNAME);
  psMainData->boBitmapScreenDimensions = TRUE;
  psMainData->nBitmapWidth = 512;
  psMainData->nBitmapHeight = 512;

	return psMainData;
}

void DeleteMainPersist (MainPersist * psMainData) {
	if (psMainData->psVisData) {
		DeleteVisPersist (psMainData->psVisData);
		psMainData->psVisData = NULL;
	}

  if (psMainData->szFilename) {
    g_string_free (psMainData->szFilename, TRUE);
  }
  if (psMainData->szFolder) {
    g_string_free (psMainData->szFolder, TRUE);
  }
  if (psMainData->szExportModelName) {
    g_string_free (psMainData->szExportModelName, TRUE);
  }
  if (psMainData->szExportBitmapName) {
    g_string_free (psMainData->szExportBitmapName, TRUE);
  }

	g_free (psMainData);
}

static void Realize (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	GdkGLContext * psGlContext = gtk_widget_get_gl_context (psWidget);
	GdkGLDrawable * psGlDrawable = gtk_widget_get_gl_drawable (psWidget);

	// OpenGL BEGIN
	if (!gdk_gl_drawable_gl_begin (psGlDrawable, psGlContext)) {
		return;
	}

	Realise (psMainData->psVisData);

	gdk_gl_drawable_gl_end (psGlDrawable);
	// OpenGL END
}

static void Unrealize (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	Unrealise (psMainData->psVisData);
}

static gboolean ConfigureEvent (GtkWidget * psWidget, GdkEventConfigure * psEvent, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	GLfloat fWidth;
	GLfloat fHeight;

	GdkGLContext * psGlContext = gtk_widget_get_gl_context (psWidget);
	GdkGLDrawable * psGlDrawable = gtk_widget_get_gl_drawable (psWidget);

	fWidth = psWidget->allocation.width;
	fHeight = psWidget->allocation.height;

	// OpenGL BEGIN
	if (!gdk_gl_drawable_gl_begin (psGlDrawable, psGlContext)) {
		return FALSE;
	}

	Reshape (fWidth, fHeight, psMainData->psVisData);

	gdk_gl_drawable_gl_end (psGlDrawable);
	// OpenGL END

	return TRUE;
}

static gboolean ExposeEvent (GtkWidget * psWidget, GdkEventExpose * psEvent, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	GdkGLContext * psGlContext = gtk_widget_get_gl_context (psWidget);
	GdkGLDrawable * psGlDrawable = gtk_widget_get_gl_drawable (psWidget);

	// OpenGL BEGIN
	if (!gdk_gl_drawable_gl_begin (psGlDrawable, psGlContext)) {
		return FALSE;
	}

	// Redraw the visualisation
	Redraw (psMainData->psVisData);

	// Swap buffers
	if (gdk_gl_drawable_is_double_buffered (psGlDrawable)) {
		gdk_gl_drawable_swap_buffers (psGlDrawable);
	}
	else {
		glFlush ();
	}

	gdk_gl_drawable_gl_end (psGlDrawable);
	// OpenGL END

	return TRUE;
}

static gboolean ButtonPressEvent (GtkWidget * psWidget, GdkEventButton * psEvent, gpointer * psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	gboolean boFullScreen;
	GtkWidget * psWidgetSet;

	boFullScreen = GetFullScreen (psMainData->psVisData);

	if (boFullScreen) {
		if (psEvent->y > (psWidget->allocation.height - FULLSCREEN_BUTTONBARSHOW)) {
			if (psMainData->boButtonBarHidden) {
				psWidgetSet = glade_xml_get_widget (psMainData->psXML, "ButtonBar");
				gtk_widget_show (psWidgetSet);
				psMainData->boButtonBarHidden = FALSE;
			}
		}
		else {
			if (!psMainData->boButtonBarHidden) {
				psWidgetSet = glade_xml_get_widget (psMainData->psXML, "ButtonBar");
				gtk_widget_hide (psWidgetSet);
				psMainData->boButtonBarHidden = TRUE;
			}
		}
	}

	Mouse (psEvent->button, psEvent->type, psEvent->x, psEvent->y, psMainData->psVisData);

	return FALSE;
}

static gboolean ButtonReleaseEvent (GtkWidget * psWidget, GdkEventButton * psEvent, gpointer * psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	Mouse (psEvent->button, psEvent->type, psEvent->x, psEvent->y, psMainData->psVisData);

	return FALSE;
}

static gboolean MotionNotifyEvent (GtkWidget * psWidget, GdkEventMotion * psEvent, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	//float fWidth = psWidget->allocation.width;
	//float fHeight = psWidget->allocation.height;
	float fX = psEvent->x;
	float fY = psEvent->y;

	Motion ((int)fX, (int)fY, psMainData->psVisData);
	gdk_window_invalidate_rect (psWidget->window, & psWidget->allocation, FALSE);

	return TRUE;
}

static gboolean KeyPressEvent (GtkWidget * psWidget, GdkEventKey * psEvent, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	guint uModifiers;

	uModifiers = gtk_accelerator_get_default_mod_mask ();

	switch (psEvent->keyval) {
		case GDK_Escape:
			gtk_main_quit ();
			break;
		case GDK_Return:
			if ((psEvent->state & uModifiers) == GDK_MOD1_MASK) {
				ToggleFullScreenWindow (psMainData);
			}
			break;
		default:
			Key (psEvent->keyval, 0, 0, (psEvent->state & uModifiers), psMainData->psVisData);
			break;
	}

	return TRUE;
}

static gboolean KeyReleaseEvent (GtkWidget * psWidget, GdkEventKey * psEvent, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	guint uModifiers;

	uModifiers = gtk_accelerator_get_default_mod_mask ();

	KeyUp (psEvent->keyval, 0, 0, (psEvent->state & uModifiers), psMainData->psVisData);

	return TRUE;
}

static gboolean ScrollEvent (GtkWidget * psWidget, GdkEventScroll * psEvent, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	GtkWidget * psWidgetSet;
	float fRadius;

	switch (psEvent->direction) {
		case GDK_SCROLL_UP:
			psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Zoom");
			fRadius = gtk_spin_button_get_value (GTK_SPIN_BUTTON (psWidgetSet));
			fRadius += RADIUS_SCROLL;
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), fRadius);
			break;
		case GDK_SCROLL_DOWN:
			psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Zoom");
			fRadius = gtk_spin_button_get_value (GTK_SPIN_BUTTON (psWidgetSet));
			fRadius -= RADIUS_SCROLL;
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), fRadius);
			break;
		default:
			// Do nothing
			break;
	}

	return TRUE;
}

void ToggleFullScreenWindow (MainPersist * psMainData) {
	GtkWidget * psWindow;
	GtkWidget * psWidget;
	gboolean boCurrentState;

	boCurrentState = GetFullScreen (psMainData->psVisData);

	if (boCurrentState) {
		psWindow = glade_xml_get_widget (psMainData->psXML, "MainWindow");
		gtk_window_unfullscreen (GTK_WINDOW (psWindow));
		psWidget = glade_xml_get_widget (psMainData->psXML, "ButtonBar");
		gtk_widget_show (psWidget);
		psMainData->boButtonBarHidden = FALSE;
	}
	else {
		psWindow = glade_xml_get_widget (psMainData->psXML, "MainWindow");
		gtk_window_fullscreen (GTK_WINDOW (psWindow));
		psWidget = glade_xml_get_widget (psMainData->psXML, "ButtonBar");
		gtk_widget_hide (psWidget);
		psMainData->boButtonBarHidden = TRUE;
	}

	psWindow = glade_xml_get_widget (psMainData->psXML, "Properties");
	gtk_window_set_keep_above (GTK_WINDOW (psWindow), !boCurrentState);
	psWindow = glade_xml_get_widget (psMainData->psXML, "Configure");
	gtk_window_set_keep_above (GTK_WINDOW (psWindow), !boCurrentState);

	ToggleFullScreen (psMainData->psVisData);
}

static gboolean Timeout (gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	if (psMainData->boTimeoutContinue) {
		Idle (psMainData->psVisData);

		// Invalidate the drawing area
		gdk_window_invalidate_rect (psMainData->psDrawingArea->window, & psMainData->psDrawingArea->allocation, 
			FALSE);

		// Update drawing area synchronously
		gdk_window_process_updates (psMainData->psDrawingArea->window, FALSE);
	}
	else {
		psMainData->TimeoutID = 0;
	}

	return psMainData->boTimeoutContinue;
}

static void TimeoutAdd (MainPersist * psMainData) {
	psMainData->boTimeoutContinue = TRUE;
  if (!psMainData->boPaused) {
		if (psMainData->TimeoutID == 0) {
			//psMainData->TimeoutID = g_timeout_add (UPDATE_TIMEOUT, Timeout, psMainData);
			psMainData->TimeoutID = g_timeout_add_full (G_PRIORITY_DEFAULT_IDLE, UPDATE_TIMEOUT, Timeout, 
				psMainData, NULL);
			//psMainData->TimeoutID = g_idle_add (Timeout, psMainData);
		}
  }
}

static void TimeoutRemove (MainPersist * psMainData) {
	psMainData->boTimeoutContinue = FALSE;
}

static gboolean MapEvent (GtkWidget * psWidget, GdkEventAny * psEvent, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	TimeoutAdd (psMainData);

	return TRUE;
}

static gboolean UnmapEvent (GtkWidget * psWidget, GdkEventAny * psEvent, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	TimeoutRemove (psMainData);

	return TRUE;
}

static gboolean VisibilityNotifyEvent (GtkWidget * psWidget, GdkEventVisibility * psEvent, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	if (psEvent->state == GDK_VISIBILITY_FULLY_OBSCURED) {
		TimeoutRemove (psMainData);
	}
	else {
		TimeoutAdd (psMainData);
	}

	return TRUE;
}

static gboolean SpinVisualisation (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	ToggleSpin (psMainData->psVisData);

	return TRUE;
}

static gboolean ConfigureGenerateSeed (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	GtkWidget * psWidgetSet;
	unsigned int uSeed;

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Seed");

	uSeed = GetRandomSeed ();
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), uSeed);

	return TRUE;
}

static gboolean ConfigureGenerateColourSeed (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	GtkWidget * psWidgetSet;
	unsigned int uSeed;

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "ColourSeed");

	uSeed = GetRandomSeed ();
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), uSeed);

	return TRUE;
}

static gboolean ConfigureDialogueOpen (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	GtkWidget * psWindow;

	SetConfigureDialogue (psMainData);

	psWindow = glade_xml_get_widget (psMainData->psXML, "Configure");
	gtk_widget_show (psWindow);

	return TRUE;
}

static gboolean ConfigureDialogueOK (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	GtkWidget * psWindow;

	SetConfigureValues (psMainData);

	psWindow = glade_xml_get_widget (psMainData->psXML, "Configure");
	gtk_widget_hide (psWindow);

	return TRUE;
}

static gboolean ConfigureDialogueApply (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;

	SetConfigureValues (psMainData);

	return TRUE;
}

void SetConfigureDialogue (MainPersist * psMainData) {
	GtkWidget * psWidgetSet;
	CelticPersist * psCelticData;
	bool boValue;
	float fValue;
	TILE eValue;
	int nValue;
	unsigned int uValue;

	nValue = GetDimensions (psMainData->psVisData);
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Dimensions2");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), (nValue == 2));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Dimensions3");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), (nValue == 3));

	psCelticData = GetCelticData (psMainData->psVisData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Symmetry");
	boValue = GetSymmetrify (psCelticData);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Invert");
	boValue = GetClearWhite (psMainData->psVisData);
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), boValue);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Seed");
	uValue = GetSeed (psCelticData);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), uValue);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "ColourSeed");
	uValue = GetColourSeed (psCelticData);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), uValue);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Width");
	nValue = GetWidth (psCelticData);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), nValue);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Height");
	nValue = GetHeight (psCelticData);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), nValue);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Depth");
	nValue = GetDepth (psCelticData);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), nValue);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Weirdness");
	fValue = GetWeirdness (psCelticData);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), fValue);

	eValue = GetOrientation (psCelticData);
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Horizontal");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), (eValue == TILE_HORIZONTAL));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Vertical");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), (eValue == TILE_VERTICAL));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Longitudinal");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetSet), (eValue == TILE_LONGITUDINAL));
}

void SetConfigureValues (MainPersist * psMainData) {
	GtkWidget * psWidgetSet;
	CelticPersist * psCelticData;
	bool boValue;
	float fValue;
	int nValue;
	unsigned int uValue;

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Dimensions2");
	boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
	if (boValue) {
		SetDimensions (2, psMainData->psVisData);
	}

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Dimensions3");
	boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
	if (boValue) {
		SetDimensions (3, psMainData->psVisData);
	}

	psCelticData = GetCelticData (psMainData->psVisData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Symmetry");
	boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
	SetSymmetrify (boValue, psCelticData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Invert");
	boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
	SetClearWhite (boValue, psMainData->psVisData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Seed");
	uValue = gtk_spin_button_get_value (GTK_SPIN_BUTTON (psWidgetSet));
	SetSeed (uValue, psCelticData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "ColourSeed");
	uValue = gtk_spin_button_get_value (GTK_SPIN_BUTTON (psWidgetSet));
	SetColourSeed (uValue, psCelticData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Width");
	nValue = gtk_spin_button_get_value (GTK_SPIN_BUTTON (psWidgetSet));
	SetWidth (nValue, psCelticData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Height");
	nValue = gtk_spin_button_get_value (GTK_SPIN_BUTTON (psWidgetSet));
	SetHeight (nValue, psCelticData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Depth");
	nValue = gtk_spin_button_get_value (GTK_SPIN_BUTTON (psWidgetSet));
	SetDepth (nValue, psCelticData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Weirdness");
	fValue = gtk_spin_button_get_value (GTK_SPIN_BUTTON (psWidgetSet));
	SetWeirdness (fValue, psCelticData);

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Horizontal");
	boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
	if (boValue) {
		SetOrientation (TILE_HORIZONTAL, psCelticData);
	}

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Vertical");
	boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
	if (boValue) {
		SetOrientation (TILE_VERTICAL, psCelticData);
	}

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Longitudinal");
	boValue = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidgetSet));
	if (boValue) {
		SetOrientation (TILE_LONGITUDINAL, psCelticData);
	}

	/* Redraw the knot */
	GenerateKnot (psCelticData);
	RenderKnots (psCelticData);
}

void SetDisplayPropertiesDialogue (MainPersist * psMainData) {
	GtkWidget * psWidget;
	float fViewRadius;
	float fTileX;
	float fTileY;
	float fTileZ;
	float fThickness;
	float fWeave;
	float fInsetX;
	float fInsetY;
	float fInsetZ;
	float fCurve;

	GetDisplayProperties (& fViewRadius, & fTileX, & fTileY, &fTileZ, & fThickness, & fWeave, & fInsetX, & fInsetY, & fInsetZ, & fCurve, psMainData->psVisData);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "Zoom");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fViewRadius);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "ZoomSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fViewRadius);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "TileX");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fTileX);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "TileXSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fTileX);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "TileY");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fTileY);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "TileYSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fTileY);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "TileZ");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fTileZ);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "TileZSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fTileZ);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "Thickness");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fThickness);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "ThicknessSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fThickness);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "Weave");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fWeave);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "WeaveSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fWeave);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetX");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fInsetX);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetXSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fInsetX);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetY");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fInsetY);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetYSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fInsetY);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetZ");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fInsetZ);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetZSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fInsetZ);

	// Spin buttons
	psWidget = glade_xml_get_widget (psMainData->psXML, "Curve");
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidget), fCurve);

	// Sliders
	psWidget = glade_xml_get_widget (psMainData->psXML, "CurveSlider");
	gtk_range_set_value (GTK_RANGE (psWidget), fCurve);
}

void SetDisplayPropertiesValues (MainPersist * psMainData) {
	GtkWidget * psWidgetSet;
	float fViewRadius;
	float fTileX;
	float fTileY;
	float fTileZ;
	float fThickness;
	float fWeave;
	float fInsetX;
	float fInsetY;
	float fInsetZ;
	float fCurve;

	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Zoom");
	fViewRadius = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "TileX");
	fTileX = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "TileY");
	fTileY = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "TileZ");
	fTileZ = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Thickness");
	fThickness = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Weave");
	fWeave = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "InsetX");
	fInsetX = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "InsetY");
	fInsetY = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "InsetZ");
	fInsetZ = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, "Curve");
	fCurve = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidgetSet));

	SetDisplayProperties (fViewRadius, fTileX, fTileY, fTileZ, fThickness, fWeave, fInsetX, fInsetY, fInsetZ, fCurve, psMainData->psVisData);
}

static gboolean DisplayPropertiesSpinChanged (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	GtkWidget * psWidgetSet;
	float fValue;
	const char * szName;
	GString * szTransfer;

	// Transfer value to slider
	szName = glade_get_widget_name (psWidget);

	szTransfer = g_string_new (szName);
	g_string_append (szTransfer, "Slider");

	fValue = gtk_spin_button_get_value_as_float (GTK_SPIN_BUTTON (psWidget));
	psWidgetSet = glade_xml_get_widget (psMainData->psXML, szTransfer->str);
	gtk_range_set_value (GTK_RANGE (psWidgetSet), fValue);

	g_string_free (szTransfer, TRUE);

	// Update the values
	SetDisplayPropertiesValues (psMainData);

	return TRUE;
}

gchar * GetTitleFilename (char const * szFilename) {
  char * szDisplayName;

  szDisplayName = g_filename_display_basename (szFilename);

  return szDisplayName;
}

void SetMainWindowTitle (char const * szFilename, MainPersist * psMainData) {
  GtkWidget * psWidget;
  GString * szTitle;
  gchar * szDisplayName;

  // Figure out what to call the window
  szDisplayName = GetTitleFilename (szFilename);
  if (strlen (szDisplayName) > 0) {
    szTitle = g_string_new ("");
    g_string_printf (szTitle, "%s - Knot", szDisplayName);
  }
  else {
    szTitle = g_string_new ("Knot");
  }

  // Set the window title
  psWidget = glade_xml_get_widget (psMainData->psXML, "MainWindow");
  gtk_window_set_title (GTK_WINDOW (psWidget), szTitle->str);

  g_free (szDisplayName);
  g_string_free (szTitle, TRUE);
}

static gboolean LoadFilePress (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
  GtkWidget * psDialogue;
  GtkWindow * psParent;
  char * szFilename;
  char * szFolder;
  bool boLoaded;
  GtkFileFilter * psFilterXML;
  GtkFileFilter * psFilterAll;
	CelticPersist * psCelticData;

  psFilterXML = gtk_file_filter_new ();
  gtk_file_filter_add_mime_type (psFilterXML, "text/xml");
  gtk_file_filter_add_mime_type (psFilterXML, "application/xml");
  gtk_file_filter_set_name (psFilterXML, "XML Files");

  psFilterAll = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (psFilterAll, "*");
  gtk_file_filter_set_name (psFilterAll, "All Files");


  psParent = GTK_WINDOW (glade_xml_get_widget (psMainData->psXML, "MainWindow"));

  psDialogue = gtk_file_chooser_dialog_new ("Open File", psParent, 
    GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
    GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

  g_object_set (G_OBJECT (psDialogue), "local-only", FALSE, NULL);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (psDialogue), psFilterXML);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (psDialogue), psFilterAll);

  if (psMainData->boFolderSet) {
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (psDialogue), psMainData->szFolder->str);
  }

  PauseAnimation (TRUE, psMainData);

  if (gtk_dialog_run (GTK_DIALOG (psDialogue)) == GTK_RESPONSE_ACCEPT) {
    szFilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (psDialogue));
    boLoaded = LoadFile (szFilename, psMainData);

    if (boLoaded) {
			psCelticData = GetCelticData (psMainData->psVisData);
			GenerateKnot (psCelticData);
			RenderKnots (psCelticData);

      g_string_assign (psMainData->szFilename, szFilename);
			SetDisplayPropertiesDialogue (psMainData);

      szFolder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (psDialogue));
      psMainData->boFolderSet = (szFolder != NULL);
      if (szFolder) {
        g_string_assign (psMainData->szFolder, szFolder);
        g_free (szFolder);
        szFolder = NULL;
      }

      // Set the window title using the file display name
      SetMainWindowTitle (szFilename, psMainData);
    }
    psMainData->boFileLoaded = boLoaded;
    g_free (szFilename);
    szFilename = NULL;
  }
  gtk_widget_destroy (psDialogue);

  PauseAnimation (FALSE, psMainData);

  return TRUE;
}

static gboolean SaveFilePress (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
  GtkWidget * psDialogue;
  GtkWindow * psParent;
  char * szFilename;
  char * szFolder;
  bool boSaved;
  GtkFileFilter * psFilterXML;
  GtkFileFilter * psFilterAll;

  psFilterXML = gtk_file_filter_new ();
  gtk_file_filter_add_mime_type (psFilterXML, "text/xml");
  gtk_file_filter_add_mime_type (psFilterXML, "application/xml");
  gtk_file_filter_set_name (psFilterXML, "XML Files");

  psFilterAll = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (psFilterAll, "*");
  gtk_file_filter_set_name (psFilterAll, "All Files");

  psParent = GTK_WINDOW (glade_xml_get_widget (psMainData->psXML, "MainWindow"));

  psDialogue = gtk_file_chooser_dialog_new ("Save File", psParent, 
    GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (psDialogue), TRUE);
  g_object_set (G_OBJECT (psDialogue), "local-only", FALSE, NULL);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (psDialogue), psFilterXML);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (psDialogue), psFilterAll);

  PauseAnimation (TRUE, psMainData);

  if (psMainData->boFolderSet) {
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (psDialogue), psMainData->szFolder->str);
  }
  else {
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (psDialogue), g_get_home_dir ());
  }

  if (!psMainData->boFileLoaded) {
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (psDialogue), DEFAULT_FILENAME);
  }
  else {
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (psDialogue), psMainData->szFilename->str);
  }

  if (gtk_dialog_run (GTK_DIALOG (psDialogue)) == GTK_RESPONSE_ACCEPT) {
    szFilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (psDialogue));
    boSaved = SaveFile (szFilename, psMainData);
    if (boSaved) {
      g_string_assign (psMainData->szFilename, szFilename);

      szFolder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (psDialogue));
      psMainData->boFolderSet = (szFolder != NULL);
      if (szFolder) {
        g_string_assign (psMainData->szFolder, szFolder);
        g_free (szFolder);
        szFolder = NULL;
      }

      // Set the window title using the file display name
      SetMainWindowTitle (szFilename, psMainData);
    }
    psMainData->boFileLoaded = boSaved;
    g_free (szFilename);
    szFilename = NULL;
  }
  gtk_widget_destroy (psDialogue);

  PauseAnimation (FALSE, psMainData);

  return TRUE;
}

static gboolean ExportModelPress (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
  GtkWidget * psDialogue;
  GtkWindow * psParent;
  char * szFilename;
  char * szFolder;
  bool boExported;
  GtkFileFilter * psFilterXML;
  GtkFileFilter * psFilterAll;
  GtkWidget * psBinary;

  psFilterXML = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (psFilterXML, "*.ply");
  gtk_file_filter_add_mime_type (psFilterXML, "text/ply");
  gtk_file_filter_add_mime_type (psFilterXML, "application/ply");
  gtk_file_filter_set_name (psFilterXML, "Stanford Triangle Format");

  psFilterAll = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (psFilterAll, "*");
  gtk_file_filter_set_name (psFilterAll, "All Files");

  psParent = GTK_WINDOW (glade_xml_get_widget (psMainData->psXML, "MainWindow"));

  psDialogue = gtk_file_chooser_dialog_new ("Export Model", psParent, 
    GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (psDialogue), TRUE);
  g_object_set (G_OBJECT (psDialogue), "local-only", FALSE, NULL);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (psDialogue), psFilterXML);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (psDialogue), psFilterAll);

  if (psMainData->boFolderSet) {
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (psDialogue), psMainData->szFolder->str);
  }
  else {
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (psDialogue), g_get_home_dir ());
  }

  if (!psMainData->boExportedModel) {
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (psDialogue), DEFAULT_EXPORTMODELNAME);
  }
  else {
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (psDialogue), psMainData->szExportModelName->str);
  }

	psBinary = gtk_check_button_new_with_label ("Binary format");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psBinary), psMainData->boBinary);
	gtk_widget_show (psBinary);
	gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (psDialogue), psBinary);

  if (gtk_dialog_run (GTK_DIALOG (psDialogue)) == GTK_RESPONSE_ACCEPT) {
  	psMainData->boBinary = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psBinary));
    szFilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (psDialogue));
    boExported = ExportModelFile (szFilename, psMainData);
    if (boExported) {
      g_string_assign (psMainData->szExportModelName, szFilename);

      szFolder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (psDialogue));
      psMainData->boFolderSet = (szFolder != NULL);
      if (szFolder) {
        g_string_assign (psMainData->szFolder, szFolder);
        g_free (szFolder);
        szFolder = NULL;
      }
    }
    psMainData->boExportedModel = boExported;
    g_free (szFilename);
    szFilename = NULL;
  }
  gtk_widget_destroy (psDialogue);

  return TRUE;
}

static gboolean BitmapWindowSizeToggle (GtkWidget * psWidget, gpointer psData) {
	BitmapSize * psBitmapSizeData = (BitmapSize * )psData;
	bool boWindowSize;
	
	boWindowSize = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (psWidget));
	
	gtk_widget_set_sensitive (psBitmapSizeData->psWidth, !boWindowSize);
	gtk_widget_set_sensitive (psBitmapSizeData->psWidthLabel, !boWindowSize);
	gtk_widget_set_sensitive (psBitmapSizeData->psHeight, !boWindowSize);
	gtk_widget_set_sensitive (psBitmapSizeData->psHeightLabel, !boWindowSize);

	return TRUE;
}



static gboolean ExportBitmapPress (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
  GtkWidget * psDialogue;
  GtkWindow * psParent;
  char * szFilename;
  char * szFolder;
  bool boExported;
  GtkFileFilter * psFilterPNG;
  GtkFileFilter * psFilterAll;
  BitmapSize sSizeWidgets;
  GtkWidget * psWidgetAdd;
  GtkWidget * psOptions;
  GtkWidget * psAlign;

  psFilterPNG = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (psFilterPNG, "*.png");
  gtk_file_filter_add_mime_type (psFilterPNG, "image/png");
  gtk_file_filter_add_mime_type (psFilterPNG, "application/png");
  gtk_file_filter_set_name (psFilterPNG, "Portable Network Graphics");

  psFilterAll = gtk_file_filter_new ();
  gtk_file_filter_add_pattern (psFilterAll, "*");
  gtk_file_filter_set_name (psFilterAll, "All Files");

  psParent = GTK_WINDOW (glade_xml_get_widget (psMainData->psXML, "MainWindow"));

  psDialogue = gtk_file_chooser_dialog_new ("Export Bitmap", psParent, 
    GTK_FILE_CHOOSER_ACTION_SAVE, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, 
    GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT, NULL);

  gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (psDialogue), TRUE);
  g_object_set (G_OBJECT (psDialogue), "local-only", FALSE, NULL);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (psDialogue), psFilterPNG);
  gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (psDialogue), psFilterAll);

  if (psMainData->boFolderSet) {
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (psDialogue), psMainData->szFolder->str);
  }
  else {
    gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (psDialogue), g_get_home_dir ());
  }

  if (!psMainData->boExportedBitmap) {
    gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (psDialogue), DEFAULT_EXPORTBITMAPNAME);
  }
  else {
    gtk_file_chooser_set_filename (GTK_FILE_CHOOSER (psDialogue), psMainData->szExportBitmapName->str);
  }

	psOptions = gtk_hbox_new (FALSE, 8);

	psAlign = gtk_alignment_new (0.0, 0.5, 0, 0);
	gtk_box_pack_start (GTK_BOX (psOptions), psAlign, FALSE, TRUE, 0);
	gtk_alignment_set_padding (GTK_ALIGNMENT (psAlign), 0, 0, 0, 20);
	gtk_widget_show(psAlign);

	psWidgetAdd = gtk_check_button_new_with_label ("Window size");
	gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (psWidgetAdd), psMainData->boBitmapScreenDimensions);
	gtk_container_add (GTK_CONTAINER (psAlign), psWidgetAdd);
	gtk_widget_show (psWidgetAdd);
	sSizeWidgets.psWindowSize = psWidgetAdd;

	psAlign = gtk_alignment_new (1.0, 0.5, 0, 0);
	gtk_box_pack_start (GTK_BOX (psOptions), psAlign, FALSE, TRUE, 0);
	gtk_widget_show(psAlign);

	psWidgetAdd = gtk_label_new ("Width");
	gtk_container_add (GTK_CONTAINER (psAlign), psWidgetAdd);
	gtk_widget_show (psWidgetAdd);
	sSizeWidgets.psWidthLabel = psWidgetAdd;

	psAlign = gtk_alignment_new (0.0, 0.5, 0, 0);
	gtk_box_pack_start (GTK_BOX (psOptions), psAlign, FALSE, TRUE, 0);
	gtk_widget_show(psAlign);

	psWidgetAdd = gtk_spin_button_new_with_range (1, 2048, 1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetAdd), psMainData->nBitmapWidth);
	gtk_container_add (GTK_CONTAINER (psAlign), psWidgetAdd);
	gtk_widget_show (psWidgetAdd);
	sSizeWidgets.psWidth = psWidgetAdd;

	psAlign = gtk_alignment_new (1.0, 0.5, 0, 0);
	gtk_box_pack_start (GTK_BOX (psOptions), psAlign, FALSE, TRUE, 0);
	gtk_widget_show(psAlign);

	psWidgetAdd = gtk_label_new ("Height");
	gtk_container_add (GTK_CONTAINER (psAlign), psWidgetAdd);
	gtk_widget_show (psWidgetAdd);
	sSizeWidgets.psHeightLabel = psWidgetAdd;

	psAlign = gtk_alignment_new (0.0, 0.5, 0, 0);
	gtk_box_pack_start (GTK_BOX (psOptions), psAlign, FALSE, TRUE, 0);
	gtk_widget_show(psAlign);

	psWidgetAdd = gtk_spin_button_new_with_range (1, 2048, 1);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetAdd), psMainData->nBitmapHeight);
	gtk_container_add (GTK_CONTAINER (psAlign), psWidgetAdd);
	gtk_widget_show (psWidgetAdd);
	sSizeWidgets.psHeight = psWidgetAdd;

	g_signal_connect (sSizeWidgets.psWindowSize, "toggled", G_CALLBACK (BitmapWindowSizeToggle), (gpointer)(& sSizeWidgets));

	gtk_widget_show (psOptions);
	gtk_file_chooser_set_extra_widget (GTK_FILE_CHOOSER (psDialogue), psOptions);

	BitmapWindowSizeToggle (sSizeWidgets.psWindowSize, (gpointer)(& sSizeWidgets));

  if (gtk_dialog_run (GTK_DIALOG (psDialogue)) == GTK_RESPONSE_ACCEPT) {

		psMainData->boBitmapScreenDimensions = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sSizeWidgets.psWindowSize));

		if (psMainData->boBitmapScreenDimensions) {
			psMainData->nBitmapWidth = GetScreenWidth (psMainData->psVisData);
			psMainData->nBitmapHeight = GetScreenHeight (psMainData->psVisData);
		}
		else {
			psMainData->nBitmapWidth = gtk_spin_button_get_value (GTK_SPIN_BUTTON (sSizeWidgets.psWidth));
			psMainData->nBitmapHeight = gtk_spin_button_get_value (GTK_SPIN_BUTTON (sSizeWidgets.psHeight));
		}

    szFilename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (psDialogue));

    boExported = ExportBitmapFile (szFilename, psMainData->nBitmapHeight, psMainData->nBitmapWidth, psMainData);

    if (boExported) {
      g_string_assign (psMainData->szExportBitmapName, szFilename);

      szFolder = gtk_file_chooser_get_current_folder (GTK_FILE_CHOOSER (psDialogue));
      psMainData->boFolderSet = (szFolder != NULL);
      if (szFolder) {
        g_string_assign (psMainData->szFolder, szFolder);
        g_free (szFolder);
        szFolder = NULL;
      }
    }
    psMainData->boExportedBitmap = boExported;
    g_free (szFilename);
    szFilename = NULL;
  }
  else {
		psMainData->boBitmapScreenDimensions = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (sSizeWidgets.psWindowSize));
		psMainData->nBitmapWidth = gtk_spin_button_get_value (GTK_SPIN_BUTTON (sSizeWidgets.psWidth));
		psMainData->nBitmapHeight = gtk_spin_button_get_value (GTK_SPIN_BUTTON (sSizeWidgets.psHeight));
  }

  gtk_widget_destroy (psDialogue);

  return TRUE;
}

static gboolean DisplayPropertiesSliderChanged (GtkWidget * psWidget, gpointer psData) {
	MainPersist * psMainData = (MainPersist * )psData;
	GtkWidget * psWidgetSet;
	float fValue;
	const char * szName;
	GString * szTransfer;
	int nLength;

	// Transfer value to spin button
	szName = glade_get_widget_name (psWidget);

	szTransfer = g_string_new (szName);
	nLength = szTransfer->len - sizeof ("Slider") + 1;
	if (nLength > 0) {
		g_string_truncate (szTransfer, nLength);

		fValue = gtk_range_get_value (GTK_RANGE (psWidget));
		psWidgetSet = glade_xml_get_widget (psMainData->psXML, szTransfer->str);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (psWidgetSet), fValue);
	}

	g_string_free (szTransfer, TRUE);

	// The values will be updated if the Spin Button value changes

	return TRUE;
}

void PauseAnimation (bool boPause, MainPersist * psMainData) {
  psMainData->boPaused = boPause;
  if (boPause) {
    TimeoutRemove (psMainData);
  }
  else {
    TimeoutAdd (psMainData);
  }
}

int main (int argc, char *argv[]) {
	GdkGLConfig * GlConfig;
	GtkWidget * psWindow;
	GtkWidget * psWidget;
	VisPersist * psVisData;
	MainPersist * psMainData;

	// Initialise various libraries
	gtk_init (&argc, &argv);
	gtk_gl_init (&argc, &argv);
	glutInit (&argc, argv);

	// Create new persistent structures
	psVisData = NewVisPersist ();
	psMainData = NewMainPersist ();
	psMainData->psVisData = psVisData;

	// Initialise visualisation
	Init (psVisData);

	// First try double buffering
	GlConfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH	| GDK_GL_MODE_DOUBLE | GDK_GL_MODE_ALPHA);
	if (GlConfig == NULL) {
		g_print ("*** Cannot find the double-buffered visual.\n");
		g_print ("*** Trying single-buffered visual.\n");

		// If that fails, we'll try single buffered
		GlConfig = gdk_gl_config_new_by_mode (GDK_GL_MODE_RGBA | GDK_GL_MODE_DEPTH | GDK_GL_MODE_ALPHA);
		if (GlConfig == NULL) {
			g_print ("*** No appropriate OpenGL-capable visual found.\n");
			exit (1);
		}
	}

	// Load the glade interface
	psMainData->psXML = glade_xml_new (KNOTDIR "/application.glade", NULL, NULL);

	// Main window
	psWindow = glade_xml_get_widget (psMainData->psXML, "MainWindow");
	gtk_container_set_reallocate_redraws (GTK_CONTAINER (psWindow), TRUE);
	psMainData->psDrawingArea = glade_xml_get_widget (psMainData->psXML, "DrawingArea");

	// Load settings
	LoadSettingsAll (psMainData);

	SetDisplayPropertiesDialogue (psMainData);

	// Automatically redraw the window children change allocation
	gtk_container_set_reallocate_redraws (GTK_CONTAINER (psWindow), TRUE);

	// Set OpenGL-capability to the drawing area widget
	gtk_widget_set_gl_capability (psMainData->psDrawingArea, GlConfig, NULL, TRUE, GDK_GL_RGBA_TYPE);

	// Connect up relevant signals
	glade_xml_signal_autoconnect (psMainData->psXML);

	g_signal_connect_after (G_OBJECT (psMainData->psDrawingArea), "realize", G_CALLBACK (Realize), (gpointer)psMainData);
	g_signal_connect_after (G_OBJECT (psMainData->psDrawingArea), "unrealize", G_CALLBACK (Unrealize), (gpointer)psMainData);

	g_signal_connect (G_OBJECT (psMainData->psDrawingArea), "configure_event", G_CALLBACK (ConfigureEvent), (gpointer)psMainData);
	g_signal_connect (G_OBJECT (psMainData->psDrawingArea), "expose_event", G_CALLBACK (ExposeEvent), (gpointer)psMainData);

	g_signal_connect (G_OBJECT (psMainData->psDrawingArea), "button_press_event", G_CALLBACK (ButtonPressEvent), (gpointer)psMainData);
	g_signal_connect (G_OBJECT (psMainData->psDrawingArea), "button_release_event", G_CALLBACK (ButtonReleaseEvent), (gpointer)psMainData);
	g_signal_connect (G_OBJECT (psMainData->psDrawingArea), "motion_notify_event", G_CALLBACK (MotionNotifyEvent), (gpointer)psMainData);
	g_signal_connect (G_OBJECT (psMainData->psDrawingArea), "scroll_event", G_CALLBACK (ScrollEvent), (gpointer)psMainData);

	g_signal_connect (G_OBJECT (psMainData->psDrawingArea), "map_event", G_CALLBACK (MapEvent), (gpointer)psMainData);
	g_signal_connect (G_OBJECT (psMainData->psDrawingArea), "unmap_event", G_CALLBACK (UnmapEvent), (gpointer)psMainData);
	g_signal_connect (G_OBJECT (psMainData->psDrawingArea), "visibility_notify_event", G_CALLBACK (VisibilityNotifyEvent), (gpointer)psMainData);

	g_signal_connect (G_OBJECT (psWindow), "key_press_event", G_CALLBACK (KeyPressEvent), (gpointer)psMainData);
	g_signal_connect (G_OBJECT (psWindow), "key_release_event", G_CALLBACK (KeyReleaseEvent), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "OpenConfigure");
	g_signal_connect (psWidget, "clicked", G_CALLBACK (ConfigureDialogueOpen), (gpointer)psMainData);
	psWidget = glade_xml_get_widget (psMainData->psXML, "ConfigureOK");
	g_signal_connect (psWidget, "clicked", G_CALLBACK (ConfigureDialogueOK), (gpointer)psMainData);
	psWidget = glade_xml_get_widget (psMainData->psXML, "ConfigureApply");
	g_signal_connect (psWidget, "clicked", G_CALLBACK (ConfigureDialogueApply), (gpointer)psMainData);
	psWidget = glade_xml_get_widget (psMainData->psXML, "Spin");
	g_signal_connect (psWidget, "toggled", G_CALLBACK (SpinVisualisation), (gpointer)psMainData);
	psWidget = glade_xml_get_widget (psMainData->psXML, "Open");
	g_signal_connect (psWidget, "clicked", G_CALLBACK (LoadFilePress), (gpointer)psMainData);
	psWidget = glade_xml_get_widget (psMainData->psXML, "Save");
	g_signal_connect (psWidget, "clicked", G_CALLBACK (SaveFilePress), (gpointer)psMainData);
	psWidget = glade_xml_get_widget (psMainData->psXML, "Export");
	g_signal_connect (psWidget, "clicked", G_CALLBACK (ExportModelPress), (gpointer)psMainData);
	psWidget = glade_xml_get_widget (psMainData->psXML, "Bitmap");
	g_signal_connect (psWidget, "clicked", G_CALLBACK (ExportBitmapPress), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "Zoom");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "ZoomSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "TileX");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "TileXSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "TileY");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "TileYSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "TileZ");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "TileZSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "Thickness");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "ThicknessSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "Weave");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "WeaveSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetX");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetXSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetY");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetYSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetZ");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "InsetZSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "Curve");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSpinChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "CurveSlider");
	g_signal_connect (psWidget, "value_changed", G_CALLBACK (DisplayPropertiesSliderChanged), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "GenerateSeed");
	g_signal_connect (psWidget, "clicked", G_CALLBACK (ConfigureGenerateSeed), (gpointer)psMainData);

	psWidget = glade_xml_get_widget (psMainData->psXML, "GenerateColourSeed");
	g_signal_connect (psWidget, "clicked", G_CALLBACK (ConfigureGenerateColourSeed), (gpointer)psMainData);

	// Display the window
	gtk_widget_show (psWindow);

	// Main loop
	gtk_main ();

	// Save settings
	SaveSettingsAll (psMainData);

	// Delete persistent structures
	DeleteVisPersist (psVisData);
	psMainData->psVisData = NULL;
	DeleteMainPersist (psMainData);

	return 0;
}

bool LoadFile (char const * szFilename, MainPersist * psMainData) {
	SettingsPersist * psSettingsData = NULL;
	SettingsLoadParser * psLoadParser = NULL;
	bool boSuccess;

	psSettingsData = NewSettingsPersist ("knot", szFilename);
	psLoadParser = g_new0 (SettingsLoadParser, 1);


	psLoadParser->LoadProperty = MainLoadProperty;
	psLoadParser->LoadSectionStart = MainLoadSectionStart;
	psLoadParser->LoadSectionEnd = MainLoadSectionEnd;
	psLoadParser->psData = psMainData;

	AddParser (psLoadParser, psSettingsData);
	
	boSuccess = SettingsLoad (psSettingsData);

	RemoveParser (psSettingsData);
	g_free (psLoadParser);
	DeleteSettingsPersist (psSettingsData);

	return boSuccess;
}

bool SaveFile (char const * szFilename, MainPersist * psMainData) {
	SettingsPersist * psSettingsData = NULL;
	bool boSuccess;

	psSettingsData = NewSettingsPersist ("knot", szFilename);

	SettingsSaveStart (psSettingsData);

	SettingsStartTag (psSettingsData, "vis");
	SaveSettingsVis (psSettingsData, psMainData->psVisData);
	SettingsEndTag (psSettingsData, "vis");

	boSuccess = SettingsSaveEnd (psSettingsData);

	DeleteSettingsPersist (psSettingsData);

	return boSuccess;
}

bool ExportModelFile (char const * szFilename, MainPersist * psMainData) {
	bool boSuccess;
	CelticPersist * psCelticData;

	psCelticData = GetCelticData (psMainData->psVisData);
	boSuccess = ExportModel (szFilename, psMainData->boBinary, psCelticData);

	return boSuccess;
}

bool ExportBitmapFile (char const * szFilename, int nHeight, int nWidth, MainPersist * psMainData) {
	bool boSuccess;

	boSuccess = ExportBitmap (szFilename, "png", nHeight, nWidth, psMainData->psVisData);

	return boSuccess;
}

void SaveSettingsAll (MainPersist * psMainData) {
	SaveFile (".knot.xml", psMainData);
}

void LoadSettingsAll (MainPersist * psMainData) {
	LoadFile (".knot.xml", psMainData);
}

void MainLoadProperty (SETTINGTYPE const eType, char const * szName, void const * const psValue, void * psData, SettingsPersist * psSettingsData) {
	// Nothing to do at this level
}

void MainLoadSectionStart (char const * szName, void * psData, SettingsPersist * psSettingsData) {
	MainPersist * psMainData = (MainPersist *)psData;

	if (stricmp (szName, "vis") == 0) {
		// Move in to the vis section
		LoadSettingsStartVis (psSettingsData, psMainData->psVisData);
	}
}

void MainLoadSectionEnd (char const * szName, void * psData, SettingsPersist * psSettingsData) {
	// Nothing to do at this level
}

