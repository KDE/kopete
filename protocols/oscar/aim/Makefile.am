SUBDIRS = ui
METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/../ \
	-I$(srcdir)/ui/ \
	-I$(top_builddir)/kopete/protocols/oscar/aim/ui \
	-I$(srcdir)/../liboscar \
	$(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_aim.la

kopete_aim_la_SOURCES = aimprotocol.cpp aimaccount.cpp aimcontact.cpp aimuserinfo.cpp aimjoinchat.cpp aimchatsession.cpp

kopete_aim_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_aim_la_LIBADD  = ../libkopete_oscar.la ui/libkopeteaimui.la \
	$(top_builddir)/kopete/libkopete/libkopete.la

service_DATA = kopete_aim.desktop aim.protocol
servicedir = $(kde_servicesdir)
