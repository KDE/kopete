METASOURCES = AUTO

SUBDIRS = icons

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_latex.la kcm_kopete_latex.la

kopete_latex_la_SOURCES = latexplugin.cpp latexconfig.kcfgc latexguiclient.cpp
kopete_latex_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kopete_latex_la_LIBADD      = ../../libkopete/libkopete.la

kcm_kopete_latex_la_SOURCES = latexprefsbase.ui latexpreferences.cpp latexconfig.kcfgc
kcm_kopete_latex_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_latex_la_LIBADD  = $(LIB_KOPETECOMPAT) $(LIB_KUTILS)

service_DATA = kopete_latex.desktop
servicedir = $(kde_servicesdir)

kcm_DATA = kopete_latex_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog

bin_SCRIPTS = kopete_latexconvert.sh
kde_kcfg_DATA = latexconfig.kcfg

mydatadir = $(kde_datadir)/kopete_latex
mydata_DATA = latexchatui.rc