METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(KOPETE_COMPAT_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_accountconfig.la

kcm_kopete_accountconfig_la_SOURCES = addaccountwizardpage1.ui addaccountwizardpage2.ui addaccountwizardpage3.ui addaccountwizard.cpp kopeteaccountconfigbase.ui kopeteaccountconfig.cpp
kcm_kopete_accountconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_accountconfig_la_LIBADD  = ../../../libkopete/libkopete.la $(LIB_KOPETECOMPAT) $(LIB_KUTILS)

service_DATA = kopete_accountconfig.desktop
servicedir = $(kde_servicesdir)

# vim: set noet:

