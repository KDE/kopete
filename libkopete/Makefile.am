METASOURCES = AUTO
SUBDIRS = ui
#
# This works only with automake:
#
#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
#    -I$(srcdir)/ui -I./ui $(all_includes)
#
# This works with unsermake:
#
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
    -I$(top_srcdir)/kopete/libkopete/ui -I$(top_builddir)/kopete/libkopete/ui $(all_includes)
#
# This worls too but is not wanted:
#
#INCLUDES = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
#    -I$(srcdir)/ui -I./ui $(all_includes)

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = kopeteprotocol.cpp kopetecontact.cpp 	pluginloader.cpp kopeteplugin.cpp kopetemessagelog.cpp 	kopetemessage.cpp kopetenotifier.cpp kopeteevent.cpp 	kopetemessagemanager.cpp kopetemessagemanagerfactory.cpp 	kopetecontactlist.cpp kopetemetacontact.cpp kopeteprefs.cpp 	kopeteemoticons.cpp kopeteaway.cpp kopeteawaydialog.cpp 	kopetetransfermanager.cpp kopeteuserpreferences.cpp kopetegroup.cpp 	kopeteidentitymanager.cpp kopeteglobalawaydialog.cpp kopeteviewmanager.cpp
libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = ui/libkopeteui.la $(LIB_KDEUI) $(LIB_KIO)

