if compile_LIBKOPETE_COMPAT
COMPAT_DIR = compat
COMPAT_LIBS = compat/libkopetecompat.la
endif

include ../../admin/Doxyfile.am
DOXYGEN_REFERENCES = kio kdecore kdeui
DOXYGEN_EXCLUDE = compat

SUBDIRS = private ui $(COMPAT_DIR) . tests

METASOURCES = AUTO

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS) \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private \
	-I$(top_srcdir)/kopete/libkopete/ui $(all_includes)

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = kopeteonlinestatus.cpp kopeteprotocol.cpp \
	kopetecontact.cpp kopetepluginmanager.cpp kopeteplugin.cpp kopetemessage.cpp \
	kopetemessagemanager.cpp kopetemessagemanagerfactory.cpp kopetecontactlist.cpp \
	kopetemetacontact.cpp kopeteawaydialog.cpp kopetetransfermanager.cpp kopetegroup.cpp \
	kcautoconfigmodule.cpp kopeteaccountmanager.cpp kopeteaccount.cpp kopeteplugindataobject.cpp \
	kopetecommandhandler.cpp kopetenotifyclient.cpp kopeteaway.cpp kopeteawayaction.cpp \
	kautoconfig.cpp kopetewalletmanager.cpp kopetecontactproperty.cpp kopetepassword.cpp \
	kopeteglobal.cpp kopeteuiglobal.cpp kopetepasswordedaccount.cpp \
	kopetemimetypehandler.cpp kopetetask.cpp kopetemimesourcefactory.cpp kopetexsl.cpp \
	kopeteeventpresentation.cpp kopetenotifyevent.cpp kopetenotifydataobject.cpp kopeteblacklister.cpp \
	kopetemessageevent.cpp

libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = -lkabc ui/libkopeteui.la $(COMPAT_LIBS) $(LIB_KIO) $(LIB_XSS) $(LIBXML_LIBS) $(LIBXSLT_LIBS)

rcdir = $(kde_datadir)/kopete
rc_DATA = kopetecommandui.rc

servicetype_DATA = kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop
servicetypedir = $(kde_servicetypesdir)

# vim: set noet:

noinst_HEADERS = kopeteblacklister.h
