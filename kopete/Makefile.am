kopete_srcdir=$(top_srcdir)/kopete
kopete_builddir=$(top_builddir)/kopete

AM_CPPFLAGS = -I$(kopete_srcdir)/kopete/conf \
	-I$(kopete_srcdir)/kopete/addcontactwizard \
	-I$(kopete_builddir)/kopete/addcontactwizard \
	-I$(kopete_builddir)/libkopete/ui \
	-I$(kopete_srcdir)/libkopete/ui \
	-I$(kopete_srcdir)/kopete/contactlist \
	-I$(kopete_srcdir)/kopete/chatwindow \
	-I$(kopete_builddir)/kopete/conf \
	-I$(kopete_srcdir)/libkopete/private \
	-I$(kopete_srcdir) \
	$(KOPETE_INCLUDES) \
	$(all_includes)
KDE_ICON = AUTO
METASOURCES = AUTO

SUBDIRS = conf addcontactwizard contactlist chatwindow

bin_PROGRAMS = kopete
kopete_SOURCES = main.cpp \
	kopete.cpp kopeteiface.cpp kopeteiface.skel \
	systemtray.cpp \
	kopeteballoon.cpp kopetewindow.cpp kopeteglobalawaydialog.cpp \
	kopeteaccountstatusbaricon.cpp kopeteprotocolstatusbaricon.cpp

kopete_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH) -lktexteditor
kopete_LDADD = \
	addcontactwizard/libkopeteaddcontactwizard.la  contactlist/libkopetecontactlist.la \
	chatwindow/libkopetechatwindow.la conf/libkopeteconf.la ../libkopete/libkopete.la

mydatadir = $(kde_datadir)/kopete
mydata_DATA = kopeteui.rc eventsrc

# this is where the desktop file will go
desktopdir   = $(kde_appsdir)/Internet
desktop_DATA = kopete.desktop

# vim: set noet:

