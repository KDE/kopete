if compile_LIBKOPETE_COMPAT
COMPAT_DIR = compat
COMPAT_LIBS = compat/libkopetecompat.la
endif

SUBDIRS = private ui $(COMPAT_DIR) .

METASOURCES = AUTO

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private -I$(top_srcdir)/kopete/libkopete/ui $(all_includes)

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = kopeteonlinestatus.cpp kopeteprotocol.cpp kopetecontact.cpp kopetepluginmanager.cpp kopeteplugin.cpp \
	kopetemessage.cpp kopetemessagemanager.cpp kopetemessagemanagerfactory.cpp kopetecontactlist.cpp kopetemetacontact.cpp \
	kopeteaway.cpp kopeteawaydialog.cpp kopetetransfermanager.cpp kopetegroup.cpp \
	kopeteaccountmanager.cpp kopeteaccount.cpp kopeteplugindataobject.cpp \
	kopetecommandhandler.cpp kopetenotifyclient.cpp 
libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = -lkabc ui/libkopeteui.la $(COMPAT_LIBS) $(LIB_KIO) $(LIB_XSS)

servicetype_DATA = kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop
servicetypedir = $(kde_servicetypesdir)

# vim: set noet:

