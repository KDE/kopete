SUBDIRS = ui pixmaps libkirc
INCLUDES = -I$(srcdir)/ui -I$(srcdir)/../../libkopete \
		-I$(srcdir)/../../libkopete/ui \
		-I./ui \
		-I$(srcdir)/libkirc \
		$(all_includes)
kde_module_LTLIBRARIES = kopete_irc.la

kopete_irc_la_SOURCES     = ircaddcontactpage.cpp irccontact.cpp ircmessage.cpp ircpreferences.cpp ircprotocol.cpp ircchatview.cpp ircservermanager.cpp ircservercontact.cpp ircconsoleview.cpp irccmdparser.cpp tabcompleter.cpp irc_channel_tabwidget.cpp irc_channel_tabbar.cpp irc_view_base.cpp irchistory.cpp ircdccview.cpp ircdccsend.cpp ircdccreceive.cpp messagetransport.cpp
kopete_irc_la_METASOURCES = AUTO
kopete_irc_la_LDFLAGS     = -module $(KDE_PLUGIN) $(all_libraries) 
kopete_irc_la_LIBADD      = ../../libkopete/libkopete.la \
		./ui/libkopeteircui.la \
		./libkirc/libkirc.la \
		$(LIB_KIO)

data_DATA = irc.plugin
datadir = $(kde_datadir)/kopete
