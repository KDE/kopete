METASOURCES = AUTO
SUBDIRS = ui pixmaps psi
INCLUDES = -I$(srcdir)/../../libkopete \
	-I$(srcdir)/../../libkopete/ui \
	-I../../libkopete/ui \
	-I$(srcdir)/ui \
	-Iui \
	-Ipsi \
	-I$(srcdir)/psi \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_jabber.la

kopete_jabber_la_SOURCES = jabberprotocol.cpp jabbercontact.cpp \
	jabberprefs.cpp
kopete_jabber_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN)
kopete_jabber_la_LIBADD  = ../../libkopete/libkopete.la \
	ui/libkopetejabberui.la psi/libpsijabber.la

data_DATA = jabber.plugin
datadir = $(kde_datadir)/kopete

