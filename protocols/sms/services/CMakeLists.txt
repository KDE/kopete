########### next target ###############
include_directories( .. )
set( kopete_sms_services_SRCS
  gsmlib.cpp
  kopete_unix_serial.cpp
  smsclient.cpp
  smssend.cpp
  smssendprovider.cpp )
#kde4_add_ui3_files( kopete_sms_services_SRCS
#  gsmlibprefs.ui
#  smsclientprefs.ui
#  smssendprefs.ui )
#add_library( kopetesmsservices SHARED ${kopete_sms_services_SRCS} )
#target_link_libraries(kopetesmsservices  KF5::KIOCore kopete )


########### install files ###############
install(TARGETS kopetesmsservices  DESTINATION ${PLUGIN_INSTALL_DIR})





