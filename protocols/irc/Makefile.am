SUBDIRS = ui pixmaps kirc
AM_CPPFLAGS = -I$(srcdir)/ui -I$(srcdir)/../../libkopete \
		-I$(srcdir)/../../libkopete/ui \
		-I./ui \
		-I$(srcdir)/kirc \
		$(all_includes)
kde_module_LTLIBRARIES = kopete_irc.la

kopete_irc_la_SOURCES     = ircaddcontactpage.cpp ircchannelcontact.cpp irccontact.cpp ircidentity.cpp ircpreferences.cpp ircprotocol.cpp
kopete_irc_la_METASOURCES = AUTO
kopete_irc_la_LDFLAGS     = -module $(KDE_PLUGIN) $(all_libraries) 
kopete_irc_la_LIBADD      = ../../libkopete/libkopete.la \
		./ui/libkopeteircui.la \
		./kirc/libkirc.la \
		$(LIB_KIO)

data_DATA = irc.desktop
datadir = $(kde_datadir)/kopete
