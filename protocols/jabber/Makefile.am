METASOURCES = AUTO
SUBDIRS = ui icons libiris
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/libiris/iris/include \
	-I$(srcdir)/libiris/iris/xmpp-im \
	-I$(srcdir)/libiris/iris/jabber \
	-I$(srcdir)/libiris/qca/src \
	-I$(srcdir)/libiris/cutestuff/util \
	-I$(srcdir)/libiris/cutestuff/network \
	-I$(srcdir)/ui \
	-I./ui \
	$(IDN_CPPFLAGS) \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_jabber.la

kopete_jabber_la_SOURCES = \
	jabberprotocol.cpp \
	jabberaccount.cpp \
	jabberresource.cpp \
	jabberresourcepool.cpp \
	jabberbasecontact.cpp \
 	jabbercontact.cpp \
	jabbergroupcontact.cpp \
	jabbergroupmembercontact.cpp \
	jabbercontactpool.cpp \
	jabberformtranslator.cpp \
	jabberformlineedit.cpp \
	jabbermessagemanager.cpp \
	jabbergroupchatmanager.cpp \
	jabberfiletransfer.cpp

kopete_jabber_la_LDFLAGS = -no-undefined -module $(IDN_LDFLAGS) $(KDE_PLUGIN) $(all_libraries)
kopete_jabber_la_LIBADD  = $(top_builddir)/kopete/libkopete/libkopete.la \
	ui/libkopetejabberui.la \
	libiris/iris/include/libiris.la \
	libiris/iris/jabber/libiris_jabber.la \
	libiris/iris/xmpp-core/libiris_xmpp_core.la \
	libiris/iris/xmpp-im/libiris_xmpp_im.la \
	libiris/qca/src/libqca.la \
	libiris/cutestuff/network/libcutestuff_network.la \
	libiris/cutestuff/util/libcutestuff_util.la

service_DATA = kopete_jabber.desktop
servicedir = $(kde_servicesdir)

noinst_HEADERS = jabberresourcepool.h jabbercontact.h jabbergroupcontact.h
