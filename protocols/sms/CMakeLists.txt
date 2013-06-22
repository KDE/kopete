add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

#add_subdirectory( ui )
#add_subdirectory( services )
add_subdirectory( icons )
include_directories( ui services )

set(kopete_sms_PART_SRCS 
  smsaddcontactpage.cpp 
  smscontact.cpp 
  smseditaccountwidget.cpp 
  smsprotocol.cpp 
  serviceloader.cpp 
  smsservice.cpp 
  smsuserpreferences.cpp 
  smsaccount.cpp
  services/gsmlib.cpp
  services/kopete_unix_serial.cpp
  services/smsclient.cpp
  services/smssend.cpp
  services/smssendprovider.cpp )

kde4_add_ui3_files( kopete_sms_PART_SRCS
  services/gsmlibprefs.ui
  services/smsclientprefs.ui
  services/smssendprefs.ui
  ui/smsactprefs.ui
  ui/smsuserprefs.ui
  ui/smsadd.ui )

kde4_add_plugin(kopete_sms ${kopete_sms_PART_SRCS})

target_link_libraries(kopete_sms  ${KDE4_KIO_LIBS} ${KDE4_KDE3SUPPORT_LIBS} kopete )

install(TARGETS kopete_sms  DESTINATION ${PLUGIN_INSTALL_DIR})
install( FILES kopete_sms.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
