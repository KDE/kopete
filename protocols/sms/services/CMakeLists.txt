########### next target ###############
include_directories( .. )
set( kopete_sms_services_SRCS
  gsmlib.cpp
  kopete_unix_serial.cpp
  smsclient.cpp
  smssend.cpp
  smssendprovider.cpp )
kde4_automoc(kopete_sms_services ${kopete_sms_services_SRCS})
kde4_add_ui3_files( kopete_sms_services_SRCS
  gsmlibprefs.ui
  smsclientprefs.ui
  smssendprefs.ui )
#kde4_add_library( kopetesmsservices SHARED ${kopete_sms_services_SRCS} )
#target_link_libraries(kopetesmsservices  ${KDE4_KIO_LIBS} kopete )


########### install files ###############
install(TARGETS kopetesmsservices  DESTINATION ${PLUGIN_INSTALL_DIR})





#original Makefile.am contents follow:

#METASOURCES = AUTO
#AM_CPPFLAGS = $(KOPETE_INCLUDES) \
#    -I$(srcdir)/.. \
#    -I.. \
#    $(all_includes)
#
#KDE_CXXFLAGS = $(USE_EXCEPTIONS)
#
#noinst_LTLIBRARIES = libkopetesmsservices.la
#
#
#libkopetesmsservices_la_SOURCES = smssend.cpp smssendprefs.ui smssendprovider.cpp \
#	smsclient.cpp smsclientprefs.ui gsmlib.cpp gsmlibprefs.ui kopete_unix_serial.cpp
#
#if include_smsgsm
#libkopetesmsservices_la_LIBADD = -lgsmme
#endif
#
