METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/msn $(all_includes)

kde_module_LTLIBRARIES = kopete_netmeeting.la

kopete_netmeeting_la_SOURCES     = netmeetingplugin.cpp netmeetinginvitation.cpp netmeetingguiclient.cpp
kopete_netmeeting_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_netmeeting_la_LIBADD      = $(top_builddir)/kopete/libkopete/libkopete.la $(top_builddir)/kopete/protocols/msn/libkopete_msn_shared.la

service_DATA = kopete_netmeeting.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete_netmeeting
mydata_DATA = netmeetingchatui.rc

