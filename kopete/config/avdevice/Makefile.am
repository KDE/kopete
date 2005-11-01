METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) \
	-I$(top_srcdir)/kopete/libkopete/avdevice \
	-I$(top_srcdir)/kopete/libkopete/private $(KOPETE_COMPAT_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_avdeviceconfig.la

kcm_kopete_avdeviceconfig_la_SOURCES = avdeviceconfig.cpp \
	avdeviceconfig_videoconfig.ui

kcm_kopete_avdeviceconfig_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)

kcm_kopete_avdeviceconfig_la_LIBADD  = ../../../libkopete/libkopete.la \
	../../../libkopete/avdevice/libkopete_videodevice.la \
	$(LIB_KOPETECOMPAT) $(LIB_KUTILS)

service_DATA = kopete_avdeviceconfig.desktop
servicedir = $(kde_servicesdir)

kopeteicondir = $(kde_datadir)/kopete/icons
kopeteicon_ICON = AUTO

# vim: set noet:
noinst_HEADERS = avdeviceconfig_videoconfig.h
