METASOURCES = AUTO
SUBDIRS = ui services
AM_CPPFLAGS = -I$(srcdir)/ui \
	-I./ui \
	-I$(srcdir)/services \
	-I./services \
	$(KOPETE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_sms.la

kopete_sms_la_SOURCES = smsaddcontactpage.cpp smscontact.cpp smspreferences.cpp \
	smsprotocol.cpp serviceloader.cpp smsservice.cpp smsuserpreferences.cpp \
	smspreferencesbase.cpp smsglobal.cpp
kopete_sms_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_sms_la_LIBADD  = ./ui/libkopetesmsui.la \
	./services/libkopetesmsservices.la \
	../../libkopete/libkopete.la  $(LIB_KIO)

data_DATA = sms.desktop
datadir = $(kde_datadir)/kopete
