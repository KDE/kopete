INCLUDES = $(all_includes) -I$(top_srcdir)/kopete/library -I$(top_srcdir)/kopete/library/ui -I$(top_srcdir)/kopete/protocols/icq -I$(top_srcdir)/kopete/protocols/icq/kxengine
SUBDIRS = library protocols

KDE_ICON = AUTO

lib_LTLIBRARIES = libkopetemain.la
libkopetemain_la_SOURCES = main.cpp
libkopetemain_la_LDFLAGS = -lX11 -lpng -lz -lm -ljpeg $(LIB_QT) -lXext -lkdecore -lkdeui $(LIB_KFILE) -lXpm -lDCOP
libkopetemain_la_LIBADD = $(top_srcdir)/kopete/library/libkopete.la

bin_PROGRAMS = kopete
kopete_SOURCES = dummy.cpp
kopete_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kopete_LDADD = $(LIB_KFILE) libkopetemain.la
CXXFLAGS = -g -O2 -Wall
METASOURCES = AUTO

CLEANFILES = dummy.cpp

icondir = $(kde_datadir)/kopete/pics
icon_DATA = newmsg.xpm

dummy.cpp:
	echo > dummy.cpp

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kopete.pot

data_DATA = kopeteui.rc
datadir = $(kde_datadir)/kopete

# this is where the desktop file will go
desktopdir   = $(kde_appsdir)/Internet
desktop_DATA = kopete.desktop
