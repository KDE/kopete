METASOURCES = AUTO

noinst_LTLIBRARIES = libxmpp.la
INCLUDES = -I../../cutestuff/network -I../../cutestuff/util -I../../cutestuff/ssl $(all_includes)

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
