kde_module_LTLIBRARIES = libkrichtexteditpart.la
METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/kopete/addcontactwizard -I$(top_srcdir)/kopete/libkopete/private $(all_includes)

noinst_LTLIBRARIES = libkopetechatwindow.la

libkopetechatwindow_la_SOURCES = kopeteviewmanager.cpp kopeteemailwindow.cpp \
	kopetechatwindow.cpp chatview.cpp \
	ktabwidget.cpp \
	emoticonselector.cpp \
	kopeteemoticonaction.cpp

libkopetechatwindow_la_LDFLAGS = $(all_libraries)
libkopetechatwindow_la_LIBADD = ../../libkopete/libkopete.la $(LIB_KDEUI)

libkrichtexteditpart_la_SOURCES = krichtexteditpart.cpp
libkrichtexteditpart_la_LDFLAGS = -module $(KDE_PLUGIN)
libkrichtexteditpart_la_LIBADD = $(KDE_RPATH) $(all_libraries) $(LIB_KPARTS) \
        $(LIB_KSPELL) $(LIB_KDEPRINT)

partrcdir = $(kde_datadir)/krichtexteditpart
partrc_DATA = krichtexteditpartui.rc

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetechatwindow.rc kopeteemailwindow.rc


# vim: set noet:

