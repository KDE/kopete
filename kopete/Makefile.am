INCLUDES = -I$(srcdir)/../libkopete \
	-I$(srcdir)/../libkopete/ui \
	$(all_includes)

KDE_ICON = AUTO
METASOURCES = AUTO

bin_PROGRAMS = kopete
kopete_SOURCES = main.cpp \
	kopeteiface.cpp kopeteiface.skel
kopete_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kopete_LDADD = ../libkopete/libkopete.la

icondir = $(kde_datadir)/kopete/pics
icon_DATA =

datadir = $(kde_datadir)/kopete
data_DATA = kopeteui.rc eventsrc

# this is where the desktop file will go
desktopdir   = $(kde_appsdir)/Internet
desktop_DATA = kopete.desktop

