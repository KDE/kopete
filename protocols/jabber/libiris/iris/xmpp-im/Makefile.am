METASOURCES = AUTO

noinst_LTLIBRARIES = libiris_xmpp_im.la
INCLUDES = -I$(srcdir)/../include -I$(srcdir)/../xmpp-core -I$(srcdir)/../xmpp-im -I$(srcdir)/../jabber -I$(srcdir)/../../cutestuff/util -I$(srcdir)/../../cutestuff/network -I$(srcdir)/../../qca/src $(all_includes)

libiris_xmpp_im_la_SOURCES = \
	client.cpp \
	types.cpp \
	xmpp_tasks.cpp \
	xmpp_vcard.cpp \
	xmpp_xmlcommon.cpp

CLEANFILES = types.moc
types.lo: types.moc
types.moc: $(top_builddir)/kopete/protocols/jabber/libiris/iris/xmpp-im/Makefile
	${MOC} -o types.moc $(srcdir)/../xmpp-im/types.cpp

KDE_OPTIONS = nofinal

