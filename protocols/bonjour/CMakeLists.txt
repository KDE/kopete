add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
)


########### next target ###############

set(kopete_bonjour_PART_SRCS 
   bonjourprotocol.cpp 
   bonjourcontact.cpp 
   bonjouraccount.cpp 
   bonjouraddcontactpage.cpp 
   bonjoureditaccountwidget.cpp 
   bonjourfakeserver.cpp 
   bonjourincomingmessage.cpp 
   ui/bonjourwebcamdialog.cpp 
)

kde4_add_ui_files(kopete_bonjour_PART_SRCS
bonjouraddui.ui
)

kde4_add_ui_files(kopete_bonjour_PART_SRCS
bonjouraccountpreferences.ui
)

kde4_add_plugin(kopete_bonjour ${kopete_bonjour_PART_SRCS})

if(WIN32)
  target_link_libraries(kopete_bonjour  ${KDE4_KIO_LIBS} ${KDE4_KDNSSD_LIBS} kopete )
else(WIN32)
  target_link_libraries(kopete_bonjour  ${KDE4_KIO_LIBS} ${KDE4_KDNSSD_LIBS} kopete kopete_videodevice )
endif(WIN32)

install(TARGETS kopete_bonjour  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_bonjour.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
