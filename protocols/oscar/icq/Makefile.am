SUBDIRS = ui .
METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/../ \
	-I$(srcdir)/ui/ \
	-I$(top_builddir)/kopete/protocols/oscar/icq/ui \
	-I$(srcdir)/../liboscar \
	$(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_icq.la

kopete_icq_la_SOURCES = icqpresence.cpp icqaccount.cpp icqcontact.cpp icqprotocol.cpp
# icquserinfo.cpp icqreadaway.cpp icqsendsmsdialog.cpp

kopete_icq_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_icq_la_LIBADD  = ../libkopete_oscar.la \
	$(top_builddir)/kopete/libkopete/libkopete.la ui/libkopeteicqui.la

service_DATA = kopete_icq.desktop
servicedir = $(kde_servicesdir)

mime_DATA = x-icq.desktop
mimedir = $(kde_mimedir)/application
