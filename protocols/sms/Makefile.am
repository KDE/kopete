METASOURCES = AUTO
SUBDIRS = ui
INCLUDES = -I$(srcdir)/ui \
	-I./ui \
	-I$(srcdir)/../../libkopete \
	-I$(srcdir)/../../libkopete/ui \
	-I../../libkopete/ui \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_sms.la

kopete_sms_la_SOURCES = smsaddcontactpage.cpp smscontact.cpp smspreferences.cpp smsprotocol.cpp smsservice.cpp
kopete_sms_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN)
kopete_sms_la_LIBADD  = ./ui/libkopetesmsui.la \
	../../libkopete/libkopete.la  $(LIB_KIO)

data_DATA = sms.plugin
datadir = $(kde_datadir)/kopete
