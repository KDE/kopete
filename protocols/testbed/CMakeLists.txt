add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
)


########### next target ###############

set(kopete_testbed_PART_SRCS 
   testbedprotocol.cpp 
   testbedcontact.cpp 
   testbedaccount.cpp 
   testbedaddcontactpage.cpp 
   testbededitaccountwidget.cpp 
   testbedfakeserver.cpp 
   testbedincomingmessage.cpp 
   ui/testbedwebcamdialog.cpp 
)

kde4_add_ui_files(kopete_testbed_PART_SRCS
testbedaddui.ui
)

kde4_add_ui_files(kopete_testbed_PART_SRCS
testbedaccountpreferences.ui
)

kde4_add_plugin(kopete_testbed ${kopete_testbed_PART_SRCS})


target_link_libraries(kopete_testbed ${KDE4_KIO_LIBS} kopete)
if(NOT DISABLE_VIDEOSUPPORT)
  target_link_libraries(kopete_testbed kopete_videodevice)
endif(NOT DISABLE_VIDEOSUPPORT)

install(TARGETS kopete_testbed  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_testbed.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
