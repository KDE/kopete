SUBDIRS = ui pixmaps
INCLUDES = -I$(srcdir)/../../libkopete \
        -I$(srcdir)/../../libkopete/ui \
	-I$(srcdir)/ui \
	-Iui \
        $(all_includes)
kde_module_LTLIBRARIES = kopete_jabber.la

kopete_jabber_la_SOURCES     = jabberprotocol.cpp kjabber.cpp jabbercontact.cpp jabberprefs.cpp jabbermessage.cpp protocol.cpp
kopete_jabber_la_METASOURCES = AUTO
kopete_jabber_la_LDFLAGS     = -module $(KDE_PLUGIN)
kopete_jabber_la_LIBADD      = ../../libkopete/libkopete.la ui/libkopetejabberui.la

data_DATA = jabber.plugin
datadir = $(kde_datadir)/kopete

