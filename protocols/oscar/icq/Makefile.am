SUBDIRS = ui .
METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/../ \
	-I$(srcdir)/ui/ \
	-I$(top_builddir)/kopete/protocols/oscar/icq/ui \
	-I$(srcdir)/../oscarsocket \
	$(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_icq.la

kopete_icq_la_SOURCES =  icqprotocol.cpp icqaccount.cpp icqcontact.cpp icquserinfo.cpp icqreadaway.cpp icqsendsmsdialog.cpp

kopete_icq_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_icq_la_LIBADD  = ../libkopete_oscar.la ui/libkopeteicqui.la \
	$(top_builddir)/kopete/libkopete/libkopete.la

#kopete_icq_la_DEPENDENCIES  = ../libkopete_oscar.la ./ui/libkopeteicqui.la \
#	$(top_builddir)/kopete/libkopete/libkopete.la

service_DATA = kopete_icq.desktop
servicedir = $(kde_servicesdir)

mime_DATA = x-icq.desktop
mimedir = $(kde_mimedir)/application
