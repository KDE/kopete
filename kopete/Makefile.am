AM_CPPFLAGS = -I$(top_srcdir)/kopete/conf \
	-I$(top_srcdir)/kopete/addcontactwizard \
	-I$(top_builddir)/kopete/addcontactwizard \
	-I$(top_builddir)/kopete/libkopete/ui \
        -I$(top_srcdir)/kopete/libkopete/ui \
        -I$(top_srcdir)/kopete/contactlist \
	-I$(top_srcdir)/kopete/chatwindow \
        -I$(top_srcdir)/kopete/conf \
        -I$(top_builddir)/kopete/conf \
        -I$(top_srcdir)/kopete/libkopete/private \
	$(KOPETE_INCLUDES) \
	$(all_includes)
KDE_ICON = AUTO
METASOURCES = AUTO

SUBDIRS = conf addcontactwizard contactlist chatwindow

bin_PROGRAMS = kopete
kopete_SOURCES = main.cpp \
	kopete.cpp kopeteiface.cpp kopeteiface.skel \
	systemtray.cpp \
	kopeteballoon.cpp kopetewindow.cpp kopeteglobalawaydialog.cpp

kopete_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH) -lktexteditor
kopete_LDADD = ../libkopete/libkopete.la conf/libkopeteconf.la \
	addcontactwizard/libkopeteaddcontactwizard.la  contactlist/libkopetecontactlist.la chatwindow/libkopetechatwindow.la $(LIBXML_LIBS) $(LIBXSLT_LIBS)

icondir = $(kde_datadir)/kopete/pics
icon_DATA =

datadir = $(kde_datadir)/kopete
data_DATA = kopeteui.rc eventsrc

# this is where the desktop file will go
desktopdir   = $(kde_appsdir)/Internet
desktop_DATA = kopete.desktop

# vim: set noet:

