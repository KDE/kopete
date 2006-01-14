METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private $(KOPETE_COMPAT_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_behaviorconfig.la

kcm_kopete_behaviorconfig_la_SOURCES = \
	kopeteawayconfigbase.ui  \
	behaviorconfig_chat.ui behaviorconfig_general.ui behaviorconfig_events.ui behaviorconfig.cpp

kcm_kopete_behaviorconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)

kcm_kopete_behaviorconfig_la_LIBADD  = ../../../libkopete/libkopete.la \
	$(LIB_KOPETECOMPAT) $(LIB_KUTILS)

service_DATA = kopete_behaviorconfig.desktop
servicedir = $(kde_servicesdir)

# vim: set noet:
