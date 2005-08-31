kopete_srcdir=$(top_srcdir)/kopete
kopete_builddir=$(top_builddir)/kopete

METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(KOPETE_COMPAT_INCLUDES) $(all_includes) -I$(kopete_srcdir)/kopete/addaccountwizard
			  

kde_module_LTLIBRARIES = kcm_kopete_accountconfig.la

kcm_kopete_accountconfig_la_SOURCES = kopeteaccountconfigbase.ui kopeteaccountconfig.cpp
kcm_kopete_accountconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kcm_kopete_accountconfig_la_LIBADD  = ../../../libkopete/libkopete.la ../../addaccountwizard/libkopeteaddaccountwizard.la $(LIB_KOPETECOMPAT) $(LIB_KUTILS)

service_DATA = kopete_accountconfig.desktop
servicedir = $(kde_servicesdir)

# vim: set noet:

