METASOURCES = AUTO

SUBDIRS = wrappers
INCLUDES = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_javascript.la kcm_kopete_javascript.la

kopete_javascript_la_SOURCES     = javascriptplugin.cpp javascriptconfig.cpp
kopete_javascript_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN )
kopete_javascript_la_LIBADD      = ../../libkopete/libkopete.la -lkjsembed \
					wrappers/libwrappers.la

kcm_kopete_javascript_la_SOURCES = javascriptprefsbase.ui javascriptpreferences.cpp \
					javascriptconfig.cpp

kcm_kopete_javascript_la_LDFLAGS = -module -no-undefined $(all_libraries)
kcm_kopete_javascript_la_LIBADD  = ../../libkopete/libkopete.la

service_DATA = kopete_javascript.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_javascript_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog
