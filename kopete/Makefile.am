kopete_srcdir=$(top_srcdir)/kopete
kopete_builddir=$(top_builddir)/kopete

INCLUDES = -I$(kopete_srcdir)/kopete

AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(kopete_srcdir)/kopete/config \
	-I$(kopete_srcdir)/kopete/addaccountwizard \
	-I$(kopete_srcdir)/kopete/addcontactwizard \
	-I$(kopete_srcdir)/kopete/config/accounts \
	-I$(kopete_builddir)/kopete/addcontactwizard \
	-I$(kopete_builddir)/libkopete/ \
	-I$(kopete_srcdir)/libkopete/ui \
	-I$(kopete_srcdir)/kopete/contactlist \
	-I$(kopete_builddir)/kopete/contactlist \
	-I$(kopete_srcdir)/kopete/chatwindow \
	-I$(kopete_srcdir)/kopete/config/plugins \
	-I$(kopete_builddir)/kopete/config \
	-I$(kopete_srcdir)/libkopete/private \
	-I$(kopete_srcdir)/libkopete/avdevice \
	-I$(kopete_srcdir) \
	$(all_includes)
KDE_ICON = AUTO
METASOURCES = AUTO

SUBDIRS = addaccountwizard addcontactwizard contactlist chatwindow config . kconf_update

bin_PROGRAMS = kopete
kopete_SOURCES = main.cpp 	kopeteapplication.cpp kopeteiface.cpp \
		kopeteiface.skel 	systemtray.cpp 	kopeteballoon.cpp kopetewindow.cpp \
			kopeteaccountstatusbaricon.cpp 	kimifaceimpl.cpp kimiface.skel kopeteeditglobalidentitywidget.cpp \
	groupkabcselectorwidget.ui

kimiface_DIR = $(kde_includes)

kopete_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH) -lktexteditor
kopete_LDADD = \
	addcontactwizard/libkopeteaddcontactwizard.la \
	addaccountwizard/libkopeteaddaccountwizard.la \
	contactlist/libkopetecontactlist.la \
	config/plugins/libkopetepluginconfig.la \
	../libkopete/libkopete.la \
	../libkopete/ui/libkopeteui.la

mydatadir    = $(kde_datadir)/kopete
mydata_DATA  = kopeteui.rc eventsrc

mimedir      = $(kde_mimedir)/application
mime_DATA    = x-kopete-emoticons.desktop

xdg_apps_DATA = kopete.desktop

# vim: set noet:
noinst_HEADERS = kimifaceimpl.h
