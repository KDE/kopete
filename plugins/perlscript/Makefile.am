AM_CPPFLAGS = -I$(srcdir)/../../libkopete -I$(srcdir)/../../libkopete/ui $(all_includes) $(LIBPERL_INCLUDES)

kde_module_LTLIBRARIES = kopete_perlscript.la

kopete_perlscript_la_SOURCES     = perlscriptplugin.cpp perlscriptprefs.cpp perlscriptprefsbase.cpp \
	addscriptdialog.cpp
kopete_perlscript_la_METASOURCES = AUTO
kopete_perlscript_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_perlscript_la_LIBADD      = ../../libkopete/libkopete.la $(LIBPERL_LIBS) -lktexteditor

data_DATA = perlscript.plugin
datadir = $(kde_datadir)/kopete
