AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes) 	-I$(top_builddir)/kopete/protocols/msn

kde_module_LTLIBRARIES = kopete_netmeeting.la

kopete_netmeeting_la_SOURCES     = netmeetingplugin.cpp netmeetinginvitation.cpp
kopete_netmeeting_la_METASOURCES = AUTO
kopete_netmeeting_la_LDFLAGS     = -module
kopete_netmeeting_la_LIBADD      = ../../libkopete/libkopete.la ../../protocols/msn/kopete_msn.la

service_DATA = netmeeting.desktop
servicedir = $(kde_servicesdir)
