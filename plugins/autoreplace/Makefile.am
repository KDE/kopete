SUBDIRS = icons
INCLUDES = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes)

kde_module_LTLIBRARIES = kopete_autoreplace.la

kopete_autoreplace_la_SOURCES     = autoreplaceplugin.cpp autoreplacepreferences.cpp autoreplaceprefs.ui
kopete_autoreplace_la_METASOURCES = AUTO
kopete_autoreplace_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN)
kopete_autoreplace_la_LIBADD      = ../../libkopete/libkopete.la

service_DATA = autoreplace.desktop
servicedir = $(kde_servicesdir)
