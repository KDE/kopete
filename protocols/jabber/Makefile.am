METASOURCES = AUTO
SUBDIRS = ui pixmaps
INCLUDES = -I$(srcdir)/../../libkopete \
	-I$(srcdir)/../../libkopete/ui \
	-I../../libkopete/ui \
	-I$(srcdir)/ui \
	-Iui \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_jabber.la

kopete_jabber_la_SOURCES = jabberprotocol.cpp jabberresource.cpp jabbercontact.cpp \
                           jabberprefs.cpp jabbermessagemanager.cpp
kopete_jabber_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN)
kopete_jabber_la_LIBADD  = ../../libkopete/libkopete.la \
	ui/libkopetejabberui.la \
	-lpsi \
	-lqssl

data_DATA = jabber.plugin
datadir = $(kde_datadir)/kopete

