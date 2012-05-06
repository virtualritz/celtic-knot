bin_PROGRAMS = ../knot
___knot_SOURCES = main.c settings.c settings.h utils.c utils.h vis.c vis.h bez.c bez.h celtic.c celtic.h celtic_private.h celtic3d.c celtic3d.h celtic2d.c celtic2d.h vecint.c vecint.h shader.c shader.h

___knot_LDADD = -lgtkglext-x11-1.0 -lgdkglext-x11-1.0 -lGLU -lglut -lXmu -lXt -lSM -lICE -lgtk-x11-2.0 -lpangox-1.0 -lX11 -lgdk-x11-2.0 -latk-1.0 -lpangoft2-1.0 -lgdk_pixbuf-2.0 -lm -lpangocairo-1.0 -lgio-2.0 -lcairo -lpango-1.0 -lfreetype -lz -lfontconfig -lgobject-2.0 -lgmodule-2.0 -lglee @KNOT_LIBS@

___knot_CPPFLAGS = -DKNOTDIR=\"$(datadir)/@PACKAGE@\" -DGL_GLEXT_PROTOTYPES=1 -Wall -I/usr/include/gtkglext-1.0 -I/usr/lib/gtkglext-1.0/include -I/usr/include/gtk-2.0 -I/usr/lib/gtk-2.0/include -I/usr/include/pango-1.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/cairo -I/usr/include/pixman-1 -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/atk-1.0 @KNOT_CFLAGS@

dist_pkgdata_DATA = ../assets/application.glade

