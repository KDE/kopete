METASOURCES = AUTO
SUBDIRS = ui
AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
    -I$(srcdir)/ui -I./ui $(all_includes)

update_DATA = kopete-pluginloader.upd kopete-pluginloader.pl
updatedir = $(kde_datadir)/kconf_update

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = kopeteprotocol.cpp kopetecontact.cpp pluginloader.cpp kopeteplugin.cpp kopetemessagelog.cpp kopetemessage.cpp kopeteevent.cpp kopetemessagemanager.cpp kopetemessagemanagerfactory.cpp kopetecontactlist.cpp kopetemetacontact.cpp kopeteprefs.cpp kopeteemoticons.cpp kopeteaway.cpp kopeteawaydialog.cpp kopetetransfermanager.cpp kopetegroup.cpp kopeteidentitymanager.cpp kopeteglobalawaydialog.cpp kopeteviewmanager.cpp kopeteidentity.cpp
libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = ui/libkopeteui.la $(LIB_KDEUI) $(LIB_KIO)

