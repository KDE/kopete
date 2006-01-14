METASOURCES = AUTO
AM_CPPFLAGS = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private \
	-I$(top_srcdir)/kopete/kopete/chatwindow $(KOPETE_COMPAT_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kcm_kopete_appearanceconfig.la

kcm_kopete_appearanceconfig_la_SOURCES = appearanceconfig_emoticons.ui \
	appearanceconfig_colors.ui appearanceconfig_chatwindow.ui appearanceconfig_contactlist.ui \
	appearanceconfig.cpp tooltipeditwidget.ui tooltipeditdialog.cpp

kcm_kopete_appearanceconfig_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) \
	$(all_libraries)
kcm_kopete_appearanceconfig_la_LIBADD = ../../../libkopete/libkopete.la  \
	../../../kopete/chatwindow/libkopetechatwindow.la \
	-lktexteditor $(LIB_KOPETECOMPAT) $(LIB_KUTILS) $(LIB_KNEWSTUFF)

service_DATA = kopete_appearanceconfig.desktop
servicedir = $(kde_servicesdir)

# vim: set noet:
