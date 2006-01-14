METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_contactnotes.la

kopete_contactnotes_la_SOURCES     = contactnotesplugin.cpp contactnotesedit.cpp
kopete_contactnotes_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_contactnotes_la_LIBADD      = ../../libkopete/libkopete.la -lkio

service_DATA = kopete_contactnotes.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete_contactnotes
mydata_DATA = contactnotesui.rc
