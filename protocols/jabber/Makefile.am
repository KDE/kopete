METASOURCES = AUTO
SUBDIRS = ui pixmaps icons psi
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(srcdir)/ui \
	-I$(top_builddir)/kopete/protocols/jabber/ui \
	-I$(srcdir)/psi/libpsi/src \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_jabber.la

kopete_jabber_la_SOURCES = jabberprotocol.cpp jabberresource.cpp jabbercontact.cpp \
                           jabberprefs.cpp jabberformtranslator.cpp \
                           jabberformlineedit.cpp jabbergroupchat.cpp \
                           jabberaccount.cpp
kopete_jabber_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN)
kopete_jabber_la_LIBADD  = ../../libkopete/libkopete.la \
	ui/libkopetejabberui.la \
	psi/libpsi/src/libpsi.la

data_DATA = jabber.desktop
datadir = $(kde_datadir)/kopete

