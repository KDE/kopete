SUBDIRS= ui
METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/ \
	-I$(srcdir)/../ \
	-I$(srcdir)/ui/ \
        -I$(builddir)/ui \
        -I$(builddir)/../aim/ui \
        -I$(builddir)/../ \
	-I$(srcdir)/../oscarsocket \
	-I$(top_builddir)/kopete/protocols/oscar/aim/ui \
	$(KOPETE_INCLUDES) \
    $(all_includes)

kde_module_LTLIBRARIES = kopete_aim.la

kopete_aim_la_SOURCES = aimprotocol.cpp aimcontact.cpp aimaccount.cpp aimuserinfo.cpp aimchangestatus.cpp

kopete_aim_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_aim_la_LIBADD  = ../libkopete_oscar.la ui/libkopeteaimui.la $(top_builddir)/kopete/libkopete/libkopete.la

service_DATA = aim.desktop
servicedir = $(kde_servicesdir)

