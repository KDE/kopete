METASOURCES = AUTO

noinst_LTLIBRARIES = libiris_xmpp_im.la
INCLUDES = -I../include -I../xmpp-core -I../xmpp-im -I../jabber -I../../cutestuff/util -I../../cutestuff/network -I../../qca/src $(all_includes)

libiris_xmpp_im_la_SOURCES = \
	client.cpp \
	types.cpp \
	xmpp_tasks.cpp \
	xmpp_vcard.cpp \
	xmpp_xmlcommon.cpp

CLEANFILES = types.moc
types.lo: types.moc
types.moc: Makefile
	moc -o types.moc types.cpp
