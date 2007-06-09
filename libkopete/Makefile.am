if compile_LIBKOPETE_COMPAT
COMPAT_DIR = compat
COMPAT_LIBS = compat/libkopetecompat.la
endif


include ../../admin/Doxyfile.am
DOXYGEN_REFERENCES = kio kdecore kdeui
DOXYGEN_EXCLUDE = compat
DOXYGEN_SET_PROJECT_NAME = libkopete

SUBDIRS = $(COMPAT_DIR) private ui . avdevice

METASOURCES = AUTO

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private \
	-I$(top_srcdir)/kopete/libkopete/ui $(all_includes)

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = knotification.cpp connectionmanager.cpp kopeteonlinestatus.cpp kopeteonlinestatusmanager.cpp \
	kopeteprotocol.cpp kopetecontact.cpp kopetepluginmanager.cpp kopeteplugin.cpp \
	kopetemessage.cpp kopetechatsession.cpp kopetechatsessionmanager.cpp \
	kopetecontactlist.cpp kopetemetacontact.cpp kopeteawaydialog.cpp kopetetransfermanager.cpp \
	kopetegroup.cpp kcautoconfigmodule.cpp kopeteaccountmanager.cpp kopeteaccount.cpp \
	kopetecontactlistelement.cpp kopetecommandhandler.cpp kopeteaway.cpp \
	kopeteawayaction.cpp kautoconfig.cpp kopetewalletmanager.cpp kopetecontactproperty.cpp \
	kopetepassword.cpp kopeteglobal.cpp kopeteuiglobal.cpp kopetepasswordedaccount.cpp \
	kopetemimetypehandler.cpp kopetetask.cpp kopetemimesourcefactory.cpp \
	kopeteeventpresentation.cpp kopetenotifyevent.cpp kopetenotifydataobject.cpp kopeteblacklister.cpp \
	kopetemessageevent.cpp kopetemessagehandler.cpp kopetemessagehandlerchain.cpp \
	kopetesimplemessagehandler.cpp kopeteproperties.cpp kabcpersistence.cpp connectionmanager.skel \
	clientiface.stub managedconnectionaccount.cpp networkstatuscommon.h kopeteconfig.kcfgc kopeteutils.cpp \
	kopeteprefs.cpp kopetepicture.cpp webcamwidget.cpp

libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = -lkabc ui/libkopeteui.la $(COMPAT_LIBS) $(LIB_KIO) $(LIB_XSS) $(LIB_XRENDER)

kde_kcfg_DATA = kopete.kcfg

#AM_CXXFLAGS = -DQT_PLUGIN
#kde_widget_LTLIBRARIES = libkopetewidgets.la
#libkopetewidgets_la_LDFLAGS = $(KDE_PLUGIN) -module $(all_libraries)
#libkopetewidgets_la_LIBADD = $(LIB_KIO) libkopete.la ui/libkopeteui.la
#libkopetewidgets_la_SOURCES = ui/kopetewidgets.cpp

kopetewidgets.cpp: $(srcdir)/kopete.widgets
	$(MAKEKDEWIDGETS) -o kopetewidgets.cpp $(srcdir)/kopete.widgets

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetecommandui.rc

servicetype_DATA = kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop
servicetypedir = $(kde_servicetypesdir)

kopeteincludedir = $(includedir)/kopete
kopeteinclude_HEADERS = kopeteaccount.h kopeteaccountmanager.h kopeteawayaction.h kopeteawaydialog.h kopeteaway.h \
	kopeteblacklister.h kopetecommandhandler.h kopetecontact.h kopetecontactlistelement.h kopetecontactlist.h \
	kopetecontactproperty.h kopeteeventpresentation.h kopete_export.h kopeteglobal.h kopetegroup.h \
	kopetemessageevent.h kopetemessage.h kopetemessagehandlerchain.h kopetemessagehandler.h \
	kopetechatsession.h kopetechatsessionmanager.h kopetemetacontact.h kopetemimetypehandler.h \
	kopeteonlinestatus.h kopeteonlinestatusmanager.h kopetepasswordedaccount.h \
	kopetepassword.h kopeteplugin.h kopeteprotocol.h kopetesimplemessagehandler.h kopetetask.h \
	kopetetransfermanager.h kopeteuiglobal.h kabcpersistence.h managedconnectionaccount.h \
	kopetenotifydataobject.h kopeteversion.h kopeteprefs.h kopetepicture.h webcamwidget.h \
	kopetepluginmanager.h

# vim: set noet:

noinst_HEADERS = kopeteblacklister.h kopeteconfig.h
