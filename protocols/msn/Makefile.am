METASOURCES = AUTO
SUBDIRS = ui . pixmaps icons config
AM_CPPFLAGS = -I$(srcdir)/ui \
	-I./ui \
	$(KOPETE_INCLUDES) \
	$(all_includes)

kde_module_LTLIBRARIES = kopete_msn.la

kopete_msn_la_SOURCES = msnprotocol.cpp msnaccount.cpp msnaddcontactpage.cpp msncontact.cpp \
	msnsocket.cpp msnauthsocket.cpp msndispatchsocket.cpp msnmessagemanager.cpp msndebugrawcmddlg.cpp \
	msnnotifysocket.cpp msnswitchboardsocket.cpp msnfiletransfersocket.cpp msninvitation.cpp
kopete_msn_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
kopete_msn_la_LIBADD  = ./ui/libkopetemsnui.la ../../libkopete/libkopete.la $(LIB_KIO)

service_DATA = msn.desktop
servicedir = $(kde_servicesdir)

mydatadir = $(kde_datadir)/kopete
mydata_DATA = msnchatui.rc

