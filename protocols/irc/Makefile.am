METASOURCES = AUTO

SUBDIRS = ui pixmaps icons libkirc
AM_CPPFLAGS = -I$(srcdir)/ui $(KOPETE_INCLUDES) \
		-I./ui \
		-I$(srcdir)/libkirc \
		$(all_includes)
kde_module_LTLIBRARIES = kopete_irc.la

kopete_irc_la_SOURCES = ircaddcontactpage.cpp ircchannelcontact.cpp \
	irccontact.cpp ircaccount.cpp  \
	ircprotocol.cpp ircservercontact.cpp ircusercontact.cpp \
	irccontactmanager.cpp kcodecaction.cpp ircguiclient.cpp ksparser.cpp

kopete_irc_la_LDFLAGS     = -module $(KDE_PLUGIN) $(all_libraries)
kopete_irc_la_LIBADD      = ../../libkopete/libkopete.la \
		./ui/libkopeteircui.la \
		./libkirc/libkirc.la \
		$(LIB_KIO)

service_DATA = kopete_irc.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete
mydata_DATA = ircchatui.rc