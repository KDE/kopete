METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_alias.la kcm_kopete_alias.la

kopete_alias_la_SOURCES     = aliasplugin.cpp
kopete_alias_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_alias_la_LIBADD      = ../../libkopete/libkopete.la

kcm_kopete_alias_la_SOURCES     = aliaspreferences.cpp aliasdialogbase.ui aliasdialog.ui editaliasdialog.cpp
kcm_kopete_alias_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_alias_la_LIBADD      = $(LIB_KOPETECOMPAT) $(LIB_KUTILS) ../../libkopete/libkopete.la

service_DATA = kopete_alias.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_alias_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

