METASOURCES = AUTO
SUBDIRS = ui services icons
AM_CPPFLAGS = -I$(srcdir)/ui \
	-I./ui \
	-I$(srcdir)/services \
	-I./services \
	$(KOPETE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_sms.la

kopete_sms_la_SOURCES = smsaddcontactpage.cpp smscontact.cpp smseditaccountwidget.cpp \
	smsprotocol.cpp serviceloader.cpp smsservice.cpp smsuserpreferences.cpp \
	smsaccount.cpp
kopete_sms_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_sms_la_LIBADD  = ./ui/libkopetesmsui.la \
	./services/libkopetesmsservices.la \
	../../libkopete/libkopete.la  $(LIB_KIO)

service_DATA = kopete_sms.desktop
servicedir = $(kde_servicesdir)
