METASOURCES = AUTO

AM_CPPFLAGS = -I$(srcdir)/../../cutestuff/network \
	-I$(srcdir)/../../cutestuff/util \
	-I$(srcdir)/../../cutestuff/ssl \
	$(SSL_INCLUDES) \
	$(all_includes)

noinst_LTLIBRARIES = libxmpp.la

libxmpp_la_LDFLAGS = -avoid-version -module

KDE_OPTIONS = nofinal

libxmpp_la_SOURCES = \
	xmpp_bytestream.cpp \
	xmpp_client.cpp \
	xmpp_dtcp.cpp \
	xmpp_ibb.cpp \
	xmpp_jid.cpp \
	xmpp_jidlink.cpp \
	xmpp_message.cpp \
	xmpp_stream.cpp \
	xmpp_tasks.cpp \
	xmpp_types.cpp \
	xmpp_vcard.cpp \
	xmpp_xmlcommon.cpp \
	xmpp_xmlfilter.cpp

