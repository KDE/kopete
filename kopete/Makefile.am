INCLUDES = -I$(srcdir)/../libkopete -I$(srcdir)/../libkopete/ui \
	-I$(srcdir)/../protocols/icq -I$(srcdir)/../protocols/icq/kxengine $(all_includes)

KDE_ICON = AUTO

lib_LTLIBRARIES = libkopetemain.la
libkopetemain_la_SOURCES = 	main.cpp \
				kopeteiface.cpp kopeteiface.skel
libkopetemain_la_LDFLAGS = -module -avoid-version $(all_libraries)
libkopetemain_la_LIBADD = ./../libkopete/libkopete.la

bin_PROGRAMS = kopete
kopete_SOURCES = dummy.cpp
kopete_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kopete_LDADD = libkopetemain.la
METASOURCES = AUTO

CLEANFILES = dummy.cpp

icondir = $(kde_datadir)/kopete/pics
icon_DATA =

dummy.cpp:
	echo > dummy.cpp

data_DATA = kopeteui.rc eventsrc
datadir = $(kde_datadir)/kopete

# this is where the desktop file will go
desktopdir   = $(kde_appsdir)/Internet
desktop_DATA = kopete.desktop



