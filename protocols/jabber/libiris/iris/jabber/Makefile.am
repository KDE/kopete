# we deal with s5b.moc separately since KDE's build system can't cope with Q_OBJECT in .cpp files
METASOURCES = filetransfer.moc xmpp_ibb.moc xmpp_jidlink.moc

noinst_LTLIBRARIES = libiris_jabber.la
INCLUDES = -I$(srcdir)/../include -I$(srcdir)/../xmpp-core -I$(srcdir)/../xmpp-im -I$(srcdir)/../../cutestuff/util -I$(srcdir)/../../cutestuff/network -I$(srcdir)/../../qca/src $(all_includes)

libiris_jabber_la_SOURCES = \
	filetransfer.cpp s5b.cpp xmpp_ibb.cpp xmpp_jidlink.cpp all_mocs.cpp

s5b.lo: s5b.moc

CLEANFILES = s5b.moc
s5b.moc: $(srcdir)/s5b.cpp $(srcdir)/s5b.h
	${MOC} $(srcdir)/s5b.h > $@
	${MOC} $(srcdir)/s5b.cpp >> $@
