SUBDIRS = ui
METASOURCES = AUTO
AM_CPPFLAGS = -I$(srcdir)/../ \
	-I$(srcdir)/ui/ \
	-I$(srcdir)/../oscarsocket \
	$(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_icq.la

kopete_icq_la_SOURCES = icqchangestatus.h icqchangestatus.cpp icqprotocol.cpp icqaccount.cpp icqcontact.cpp
	icquserinfo.cpp 

kopete_icq_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_icq_la_LIBADD  = ../kopete_oscar.la $(top_builddir)/kopete/libkopete/libkopete.la ./ui/libkopeteicqui.la

service_DATA = icq.desktop
servicedir = $(kde_servicesdir)
