kopete_srcdir=$(top_srcdir)/kopete
kopete_builddir=$(top_builddir)/kopete

AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(kopete_srcdir)/kopete/conf \
	-I$(kopete_srcdir)/kopete/addcontactwizard \
	-I$(kopete_builddir)/kopete/addcontactwizard \
	-I$(kopete_srcdir)/libkopete/ui \
	-I$(kopete_srcdir)/kopete/contactlist \
	-I$(kopete_srcdir)/kopete/chatwindow \
	-I$(kopete_srcdir)/kopete/config/plugins \
	-I$(kopete_builddir)/kopete/conf \
	-I$(kopete_srcdir)/libkopete/private \
	-I$(kopete_srcdir) \
	$(all_includes)
KDE_ICON = AUTO
METASOURCES = AUTO

SUBDIRS = addcontactwizard contactlist config . chatwindow kconf_update 

bin_PROGRAMS = kopete
kopete_SOURCES = main.cpp \
	kopeteapplication.cpp kopeteiface.cpp kopeteiface.skel \
	systemtray.cpp \
	kopeteballoon.cpp kopetewindow.cpp \
	kopeteaccountstatusbaricon.cpp

kopete_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH) -lktexteditor
kopete_LDADD = \
	addcontactwizard/libkopeteaddcontactwizard.la contactlist/libkopetecontactlist.la \
	../libkopete/libkopete.la config/plugins/libkopetepluginconfig.la

mydatadir = $(kde_datadir)/kopete
mydata_DATA = kopeteui.rc eventsrc

# please remove the following 2 lines for the KDE 3.2 release...
desktopdir   = $(kde_appsdir)/Internet
desktop_DATA = kopete.desktop

# ... and uncomment the following line instead
# xdg_apps_DATA = kopete.desktop

# vim: set noet:

