AM_CPPFLAGS = -I$(srcdir)/conf \
	-I$(srcdir)/addcontactwizard \
	-I./addcontactwizard \
	-I../libkopete/ui \
	-I$(top_srcdir)/kopete/libkopete/private \
	$(KOPETE_INCLUDES) \
	$(all_includes)
KDE_ICON = AUTO
METASOURCES = AUTO

SUBDIRS = conf addcontactwizard

bin_PROGRAMS = kopete
kopete_SOURCES = main.cpp \
	kopetewindow.cpp kopete.cpp kopeteiface.cpp kopeteiface.skel \
	kopetemetacontactlvi.cpp kopetestatusgroupviewitem.cpp kopetegroupviewitem.cpp \
	statusbaricon.cpp kopetecontactlistview.cpp systemtray.cpp \
	kopeteballoon.cpp kopeteglobalawaydialog.cpp

kopete_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH) -lktexteditor
kopete_LDADD = ../libkopete/libkopete.la conf/libkopeteconf.la \
	addcontactwizard/libkopeteaddcontactwizard.la  $(LIBXML_LIBS) $(LIBXSLT_LIBS)

icondir = $(kde_datadir)/kopete/pics
icon_DATA =

datadir = $(kde_datadir)/kopete
data_DATA = kopeteui.rc eventsrc

# this is where the desktop file will go
desktopdir   = $(kde_appsdir)/Internet
desktop_DATA = kopete.desktop

# vim: set noet:

