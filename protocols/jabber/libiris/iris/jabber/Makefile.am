# we deal with s5b.moc separately since KDE's build system can't cope with Q_OBJECT in .cpp files
METASOURCES = filetransfer.moc xmpp_ibb.moc xmpp_jidlink.moc

noinst_LTLIBRARIES = libiris_jabber.la
INCLUDES = -I$(srcdir)/../include -I$(srcdir)/../xmpp-core -I$(srcdir)/../xmpp-im -I$(srcdir)/../../cutestuff/util -I$(srcdir)/../../cutestuff/network -I$(srcdir)/../../qca/src $(all_includes)

libiris_jabber_la_SOURCES = \
	filetransfer.cpp s5b.cpp xmpp_ibb.cpp xmpp_jidlink.cpp all_mocs.cpp

CLEANFILES = s5b.moc
s5b.moc: s5b.cpp s5b.h Makefile
	${MOC} s5b.h > $@
	${MOC} s5b.cpp >> $@
