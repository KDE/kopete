
include ../../admin/Doxyfile.am
DOXYGEN_REFERENCES = kio kdecore kdeui
DOXYGEN_EXCLUDE = compat

SUBDIRS =

METASOURCES = AUTO

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS) \
	$(KOPETE_INCLUDES) $(all_includes)

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = kopetepluginmanager.cpp kopeteplugin.cpp kopeteprotocol.cpp \
	kopeteaccountmanager.cpp kopeteaccount.cpp \
	kopetecontactlist.cpp kopetecontact.cpp kopetemetacontact.cpp kopetegroup.cpp kopetecontactlistelement.cpp \
	kopeteonlinestatus.cpp kopeteonlinestatusmanager.cpp

libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = -lkabc -lkutils $(LIB_KIO) $(LIB_XSS) $(LIBXML_LIBS) $(LIBXSLT_LIBS) $(LIB_)

#rcdir = $(kde_datadir)/kopete
#rc_DATA = kopetecommandui.rc

servicetype_DATA = kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop
servicetypedir = $(kde_servicetypesdir)
