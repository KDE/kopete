METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/msn $(all_includes)

kde_module_LTLIBRARIES = kopete_netmeeting.la

kopete_netmeeting_la_SOURCES     = netmeetingplugin.cpp netmeetinginvitation.cpp netmeetingguiclient.cpp
kopete_netmeeting_la_LDFLAGS     = -module
kopete_netmeeting_la_LIBADD      = $(top_builddir)/kopete/libkopete/libkopete.la $(top_builddir)/kopete/protocols/msn/kopete_msn.la

service_DATA = netmeeting.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete
mydata_DATA = netmeetingchatui.rc

