METASOURCES = AUTO
SUBDIRS = ui pixmaps
AM_CPPFLAGS = -I$(srcdir)/../../libkopete \
	-I$(srcdir)/../../libkopete/ui \
	-I../../libkopete/ui \
	-I$(srcdir)/ui \
	-Iui \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_jabber.la

kopete_jabber_la_SOURCES = jabberprotocol.cpp jabberresource.cpp jabbercontact.cpp \
                           jabberprefs.cpp jabbermessagemanager.cpp jabberformtranslator.cpp \
                           jabberformlineedit.cpp
kopete_jabber_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN)
kopete_jabber_la_LIBADD  = ../../libkopete/libkopete.la \
	ui/libkopetejabberui.la \
	-lpsi

data_DATA = jabber.desktop
datadir = $(kde_datadir)/kopete
