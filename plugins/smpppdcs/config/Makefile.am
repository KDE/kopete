METASOURCES = AUTO

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_smpppdcs.la

kcm_kopete_smpppdcs_la_SOURCES = smpppdcsprefs.ui smpppdcspreferences.cpp \
	smpppdsearcher.cpp smpppdcsprefsimpl.cpp
kcm_kopete_smpppdcs_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_smpppdcs_la_LIBADD  = ../../../libkopete/libkopete.la $(LIB_KUTILS)

kcm_DATA = kopete_smpppdcs_config.desktop
kcmdir = $(kde_servicesdir)/kconfiguredialog
noinst_HEADERS = smpppdsearcher.h smpppdcsprefsimpl.h
