AM_CPPFLAGS = -I$(srcdir)/conf \
	-I$(srcdir)/addcontactwizard \
	-I./addcontactwizard \
	$(KOPETE_INCLUDES) \
	$(all_includes)

KDE_ICON = AUTO
METASOURCES = AUTO

SUBDIRS = conf addcontactwizard

noinst_HEADERS = \
	kopetewindow.h kopete.h \
	kopetemetacontactlvi.h kopetegroupviewitem.h kopetecontactlistview.h \
	statusbaricon.h systemtray.h kopeteballoon.h

bin_PROGRAMS = kopete
kopete_SOURCES = main.cpp \
	kopetewindow.cpp kopete.cpp kopeteiface.cpp kopeteiface.skel \
	kopetemetacontactlvi.cpp kopetestatusgroupviewitem.cpp kopetegroupviewitem.cpp \
	statusbaricon.cpp kopetecontactlistview.cpp systemtray.cpp \
	kopeteballoon.cpp

kopete_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopete_LDADD = ../libkopete/libkopete.la conf/libkopeteconf.la \
	addcontactwizard/libkopeteaddcontactwizard.la

icondir = $(kde_datadir)/kopete/pics
icon_DATA =

datadir = $(kde_datadir)/kopete
data_DATA = kopeteui.rc eventsrc

# this is where the desktop file will go
desktopdir   = $(kde_appsdir)/Internet
desktop_DATA = kopete.desktop

# vim: set noet:

