METASOURCES = AUTO
SUBDIRS = ui
INCLUDES = -I$(srcdir)/ui -I./ui $(all_includes)

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = kopete.cpp kopeteprotocol.cpp kopetecontact.cpp \
	pluginloader.cpp kopeteplugin.cpp kopetemessagelog.cpp \
	kopetemessage.cpp kopetenotifier.cpp kopeteevent.cpp \
	kopetemessagemanager.cpp kopetemessagemanagerfactory.cpp \
	kopetecontactlist.cpp kopetemetacontact.cpp kopeteprefs.cpp \
	kopeteemoticons.cpp kopeteaway.cpp kopeteawaydialog.cpp \
	kopetetransfermanager.cpp kopeteuserpreferences.cpp kopetegroup.cpp \
	kopeteidentitymanager.cpp
libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = ui/libkopeteui.la $(LIB_KDEUI) $(LIB_KIO)

