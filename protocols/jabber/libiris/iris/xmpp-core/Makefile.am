# The only Q_OBJECT lines are in securestream.{h,cpp} and we deal with them below.
# Give metasources a file with no Q_OBJECT line to stop unsermake assuming we want METASOURCES = AUTO
METASOURCES = ignore_this_warning.moc

noinst_LTLIBRARIES = libiris_xmpp_core.la
AM_CPPFLAGS = $(IDN_CPPFLAGS)
INCLUDES = -I$(srcdir)/../include -I$(srcdir)/../xmpp-core -I$(srcdir)/../xmpp-im -I$(srcdir)/../../cutestuff/util -I$(srcdir)/../../cutestuff/network -I$(srcdir)/../../qca/src $(all_includes)

libiris_xmpp_core_la_CPPFLAGS = $(IDN_CPPFLAGS)
libiris_xmpp_core_la_LDFLAGS = $(IDN_LDFLAGS) $(IDN_LIBS)
libiris_xmpp_core_la_SOURCES = \
	connector.cpp \
	jid.cpp \
	securestream.cpp \
	tlshandler.cpp \
	hash.cpp \
	protocol.cpp \
	stream.cpp \
	xmlprotocol.cpp \
	parser.cpp \
	simplesasl.cpp

#
# The rules below are hacks, the first is for unsermake-type naming schemes,
# the second one for automake-type naming schemes. We need to process this
# moc before the object gets compiled in. The additional rule will be
# ignored.
#
securestream.lo: securestream.moc
libiris_xmpp_core_la-securestream.lo: securestream.moc

CLEANFILES = securestream.moc
securestream.moc: $(srcdir)/securestream.cpp $(srcdir)/securestream.h
	${MOC} $(srcdir)/securestream.h > $@
	${MOC} $(srcdir)/securestream.cpp >> $@

KDE_OPTIONS = nofinal
