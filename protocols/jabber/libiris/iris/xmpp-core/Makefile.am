# The only Q_OBJECT lines are in securestream.{h,cpp} and we deal with them below.
# Give metasources a file with no Q_OBJECT line to stop unsermake assuming we want METASOURCES = AUTO
METASOURCES = ignore_this_warning.moc

noinst_LTLIBRARIES = libiris_xmpp_core.la
INCLUDES = -I$(srcdir)/../include -I$(srcdir)/../xmpp-core -I$(srcdir)/../xmpp-im -I$(srcdir)/../../cutestuff/util -I$(srcdir)/../../cutestuff/network -I$(srcdir)/../../qca/src $(all_includes)

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

CLEANFILES = securestream.moc
securestream.moc: securestream.cpp securestream.h Makefile
	${MOC} securestream.h > $@
	${MOC} securestream.cpp >> $@
