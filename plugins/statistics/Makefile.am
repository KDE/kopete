METASOURCES = AUTO

INCLUDES = $(KOPETE_INCLUDES) $(all_includes)

SUBDIRS = sqlite

kde_module_LTLIBRARIES = kopete_statistics.la

kopete_statistics_la_SOURCES = statisticsplugin.cpp statisticsdb.cpp statisticsdialog.cpp statisticswidget.ui statisticscontact.cpp statisticsdcopiface.skel

kopete_statistics_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_statistics_la_LIBADD      = ../../libkopete/libkopete.la sqlite/libsqlite.la

service_DATA = kopete_statistics.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete_statistics
mydata_DATA = statisticsui.rc

mydatadirimagesdir = $(kde_datadir)/kopete/pics/statistics
mydatadirimages_DATA = images/blue.png images/navy.png images/black.png images/gray.png
