METASOURCES = AUTO
SUBDIRS = ui services
AM_CPPFLAGS = -I$(top_srcdir)/kopete/protocols/sms/ui \
	-I$(top_builddir)/kopete/protocols/sms/ui \
	-I$(top_srcdir)/kopete/protocols/sms/services \
	-I$(top_builddir)/kopete/protocols/sms/services \
	-I./services \
	-I$(top_srcdir)/kopete/libkopete \
	-I$(top_srcdir)/kopete/libkopete/ui \
	-I$(top_builddir)/kopete/libkopete/ui \
	-I../../libkopete/ui \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_sms.la

kopete_sms_la_SOURCES = smsaddcontactpage.cpp smscontact.cpp smspreferences.cpp \
	smsprotocol.cpp serviceloader.cpp smsservice.cpp smsuserpreferences.cpp \
	smspreferencesbase.cpp smsglobal.cpp
kopete_sms_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_sms_la_LIBADD  = ./ui/libkopetesmsui.la \
	./services/libkopetesmsservices.la \
	../../libkopete/libkopete.la  $(LIB_KIO)

data_DATA = sms.plugin
datadir = $(kde_datadir)/kopete
