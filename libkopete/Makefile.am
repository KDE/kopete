SUBDIRS = private ui compat .

METASOURCES = AUTO

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private -I$(top_srcdir)/kopete/libkopete/ui -I$(top_srcdir)/kopete/libkopete/compat/kcdialog $(all_includes)

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = kopeteonlinestatus.cpp kopeteprotocol.cpp kopetecontact.cpp pluginloader.cpp kopeteplugin.cpp \
	kopetemessage.cpp kopetemessagemanager.cpp kopetemessagemanagerfactory.cpp kopetecontactlist.cpp kopetemetacontact.cpp \
	kopeteaway.cpp kopeteawaydialog.cpp kopetetransfermanager.cpp kopetegroup.cpp \
	kopeteaccountmanager.cpp kopeteaccount.cpp kopeteplugindataobject.cpp \
	kopetecommandhandler.cpp

libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = ui/libkopeteui.la \
	$(LIB_KOPETECOMPAT) ../libkopete/compat/kcdialog/libkopetecompat_kcdialog.la $(LIB_KIO) $(LIB_XSS)

servicetype_DATA = kopeteplugin.desktop kopeteprotocol.desktop
servicetypedir = $(kde_servicetypesdir)

# vim: set noet:
