METASOURCES = AUTO

noinst_LTLIBRARIES = libxmpp.la
INCLUDES = $(all_includes) -I../../cutestuff/network -I../../cutestuff/util -I../../cutestuff/ssl

libxmpp_la_LDFLAGS = -avoid-version -module

KDE_OPTIONS = nofinal

xmpp_types.moc: xmpp_types.cpp
	moc xmpp_types.cpp > xmpp_types.moc

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
