METASOURCES = AUTO
SUBDIRS = ui pixmaps icons libxmpp
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/libxmpp/xmpp/src \
	-I$(srcdir)/ui \
	-I./ui \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_jabber.la

kopete_jabber_la_SOURCES = jabberprotocol.cpp jabberaccount.cpp jabberresource.cpp jabbercontact.cpp                            jabberformtranslator.cpp jabberformlineedit.cpp jabbergroupchat.cpp jabberawaydialog.cpp

kopete_jabber_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_jabber_la_LIBADD  = ../../libkopete/libkopete.la \
	ui/libkopetejabberui.la \
	libxmpp/xmpp/src/libxmpp.la \
	libxmpp/cutestuff/network/libcutestuff_network.la \
	libxmpp/cutestuff/ssl/libcutestuff_ssl.la \
	libxmpp/cutestuff/util/libcutestuff_util.la

service_DATA = kopete_jabber.desktop
servicedir = $(kde_servicesdir)

