METASOURCES = AUTO
SUBDIRS = ui compat

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT -DQT_NO_CAST_ASCII -DQT_NO_ASCII_CAST \
	$(KOPETE_INCLUDES) $(all_includes)

update_DATA = kopete-pluginloader.upd kopete-pluginloader.pl
updatedir = $(kde_datadir)/kconf_update

lib_LTLIBRARIES = libkopete.la

libkopete_la_SOURCES = kopeteonlinestatus.cpp kopeteprotocol.cpp kopetecontact.cpp pluginloader.cpp kopeteplugin.cpp kopetemessagelog.cpp 	kopetemessage.cpp kopeteevent.cpp kopetemessagemanager.cpp kopetemessagemanagerfactory.cpp kopetecontactlist.cpp kopetemetacontact.cpp 	kopeteprefs.cpp kopeteemoticons.cpp kopeteaway.cpp kopeteawaydialog.cpp kopetetransfermanager.cpp kopetegroup.cpp kopeteaccountmanager.cpp kopeteviewmanager.cpp kopeteaccount.cpp kopeteplugindataobject.cpp
libkopete_la_LDFLAGS = -no-undefined -version-info 1:0:0 $(all_libraries)
libkopete_la_LIBADD = ui/libkopeteui.la $(LIB_KOPETECOMPAT) $(LIB_KIO)

servicetype_DATA = kopeteplugin.desktop kopeteprotocol.desktop
servicetypedir = $(kde_servicetypesdir)

