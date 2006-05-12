
add_subdirectory( ui ) 
add_subdirectory( services ) 
add_subdirectory( icons ) 




########### next target ###############

set(kopete_sms_PART_SRCS 
   smsaddcontactpage.cpp 
   smscontact.cpp 
   smseditaccountwidget.cpp 
   smsprotocol.cpp 
   serviceloader.cpp 
   smsservice.cpp 
   smsuserpreferences.cpp 
   smsaccount.cpp )

kde4_automoc(${kopete_sms_PART_SRCS})

kde4_add_plugin(kopete_sms ${kopete_sms_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_sms )

target_link_libraries(kopete_sms  ${KDE4_KDECORE_LIBS} kopetesmsui kopetesmsservices kopete kio )

install_targets(${PLUGIN_INSTALL_DIR} kopete_sms )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_sms.desktop )




#original Makefile.am contents follow:

#METASOURCES = AUTO
#SUBDIRS = ui services icons
#AM_CPPFLAGS = -I$(srcdir)/ui \
#	-I./ui \
#	-I$(srcdir)/services \
#	-I./services \
#	$(KOPETE_INCLUDES) \
#	$(all_includes)
#
#kde_module_LTLIBRARIES = kopete_sms.la
#
#kopete_sms_la_SOURCES = smsaddcontactpage.cpp smscontact.cpp smseditaccountwidget.cpp \
#	smsprotocol.cpp serviceloader.cpp smsservice.cpp smsuserpreferences.cpp \
#	smsaccount.cpp
#kopete_sms_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#kopete_sms_la_LIBADD  = ./ui/libkopetesmsui.la \
#	./services/libkopetesmsservices.la \
#	../../libkopete/libkopete.la  $(LIB_KIO)
#
#service_DATA = kopete_sms.desktop
#servicedir = $(kde_servicesdir)
