METASOURCES = AUTO
SUBDIRS = ui icons libiris
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/libiris/iris/include \
	-I$(srcdir)/libiris/iris/xmpp-im \
	-I$(srcdir)/libiris/qca/src \
	-I$(srcdir)/ui \
	-I./ui \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_jabber.la

kopete_jabber_la_SOURCES = jabberprotocol.cpp jabberaccount.cpp jabberresource.cpp jabbercontact.cpp                            jabberformtranslator.cpp jabberformlineedit.cpp jabbergroupchat.cpp jabberawaydialog.cpp

kopete_jabber_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_jabber_la_LIBADD  = ../../libkopete/libkopete.la \
	ui/libkopetejabberui.la \
	libiris/iris/jabber/libiris_jabber.la \
	libiris/iris/xmpp-core/libiris_xmpp_core.la \
	libiris/iris/xmpp-im/libiris_xmpp_im.la \
	libiris/qca/src/libqca.la \
	libiris/cutestuff/network/libcutestuff_network.la \
	libiris/cutestuff/util/libcutestuff_util.la
kopete_jabber_la_LDADD = -lidn

service_DATA = kopete_jabber.desktop
servicedir = $(kde_servicesdir)

