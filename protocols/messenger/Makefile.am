METASOURCES = AUTO
SUBDIRS = 
AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_messenger.la

kopete_messenger_la_SOURCES = messengerprotocol.cpp messengeraccount.cpp messengercontact.cpp
kopete_messenger_la_LIBADD = $(LIB_KIO) ../../libkopete/libkopete.la
kopete_messenger_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)

service_DATA = kopete_messenger.desktop
servicedir = $(kde_servicesdir)

#mydatadir = $(kde_datadir)/kopete_messenger
#mydata_DATA = msnchatui.rc

