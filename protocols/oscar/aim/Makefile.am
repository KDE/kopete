SUBDIRS = ui
METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/ \
	-I$(srcdir)/../ -I.. \
	-I$(srcdir)/ui/ -I./ui \
	-I../aim/ui \
	-I$(srcdir)/../oscarsocket \
	-I$(top_builddir)/kopete/protocols/oscar/aim/ui \
	$(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_aim.la

kopete_aim_la_SOURCES = aimprotocol.cpp aimcontact.cpp aimaccount.cpp aimuserinfo.cpp

kopete_aim_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_aim_la_LIBADD  = ../libkopete_oscar.la ui/libkopeteaimui.la $(top_builddir)/kopete/libkopete/libkopete.la

service_DATA = aim.desktop
servicedir = $(kde_servicesdir)

