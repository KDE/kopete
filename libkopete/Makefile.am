include ../../admin/Doxyfile.am
DOXYGEN_REFERENCES = kio kdecore kdeui
DOXYGEN_EXCLUDE = compat

SUBDIRS = private ui  . tests

METASOURCES = AUTO

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS) \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private \
	-I$(top_srcdir)/kopete/libkopete/ui $(all_includes)

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = connectionmanager.cpp kopeteonlinestatus.cpp kopeteonlinestatusmanager.cpp \
	kopeteprotocol.cpp kopetecontact.cpp kopetepluginmanager.cpp kopeteplugin.cpp \
	kopetemessage.cpp kopetemessagemanager.cpp kopetemessagemanagerfactory.cpp \
	kopetecontactlist.cpp kopetemetacontact.cpp kopeteawaydialog.cpp kopetetransfermanager.cpp \
	kopetegroup.cpp kcautoconfigmodule.cpp kopeteaccountmanager.cpp kopeteaccount.cpp \
	kopetecontactlistelement.cpp kopetecommandhandler.cpp kopetenotifyclient.cpp kopeteaway.cpp \
	kopeteawayaction.cpp kautoconfig.cpp kopetewalletmanager.cpp kopetecontactproperty.cpp \
	kopetepassword.cpp kopeteglobal.cpp kopeteuiglobal.cpp kopetepasswordedaccount.cpp \
	kopetemimetypehandler.cpp kopetetask.cpp kopetemimesourcefactory.cpp kopetexsl.cpp \
	kopeteeventpresentation.cpp kopetenotifyevent.cpp kopetenotifydataobject.cpp kopeteblacklister.cpp \
	kopetemessageevent.cpp kopetemessagehandler.cpp kopetemessagehandlerchain.cpp \
	kopetesimplemessagehandler.cpp kopeteproperties.cpp kabcpersistence.cpp connectionmanager.skel \
	clientiface.stub managedconnectionaccount.cpp networkstatuscommon.h

libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = -lkabc ui/libkopeteui.la $(LIB_KIO) $(LIB_XSS) $(LIBXML_LIBS) $(LIBXSLT_LIBS)

#AM_CXXFLAGS = -DQT_PLUGIN 
kde_widget_LTLIBRARIES = libkopetewidgets.la
widget.includeFile = QString::fromLatin1("kopete/ui/kopetelistview.h");_LDFLAGS = $(KDE_PLUGIN) -module $(all_libraries)
libkopetewidgets_la_LIBADD = $(LIB_KIO) libkopete.la ui/libkopeteui.la
libkopetewidgets_la_SOURCES = ui/kopetewidgets.cpp 

#kopetewidgets.cpp: $(srcdir)/kopete.widgets
#	$(MAKEKDEWIDGETS) -o kopetewidgets.cpp $(srcdir)/kopete.widgets

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetecommandui.rc

servicetype_DATA = kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop
servicetypedir = $(kde_servicetypesdir)

kopeteincludedir = $(includedir)/kopete
kopeteinclude_HEADERS = kopeteaccount.h kopeteaccountmanager.h kopeteawayaction.h kopeteawaydialog.h kopeteaway.h \
	kopeteblacklister.h kopetecommandhandler.h kopetecontact.h kopetecontactlistelement.h kopetecontactlist.h \
	kopetecontactproperty.h kopeteeventpresentation.h kopete_export.h kopeteglobal.h kopetegroup.h \
	kopetemessageevent.h kopetemessage.h kopetemessagehandlerchain.h kopetemessagehandler.h \
	kopetemessagemanagerfactory.h kopetemessagemanager.h kopetemetacontact.h kopetemimetypehandler.h \
	kopetenotifyclient.h kopeteonlinestatus.h kopeteonlinestatusmanager.h kopetepasswordedaccount.h \
	kopetepassword.h kopeteplugin.h kopeteprotocol.h kopetesimplemessagehandler.h kopetetask.h \
	kopetetransfermanager.h kopeteuiglobal.h kopetexsl.h kabcpersistence.h managedconnectionaccount.h

# vim: set noet:

noinst_HEADERS = kopeteblacklister.h
