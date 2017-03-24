add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
)


########### next target ###############
find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS KIO)
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

ki18n_wrap_ui(kopete_testbed_PART_SRCS
testbedaddui.ui
)

ki18n_wrap_ui(kopete_testbed_PART_SRCS
testbedaccountpreferences.ui
)

add_library(kopete_testbed MODULE ${kopete_testbed_PART_SRCS})


target_link_libraries(kopete_testbed KF5::KIOCore kopete)
if(NOT DISABLE_VIDEOSUPPORT)
  target_link_libraries(kopete_testbed kopete_videodevice)
endif(NOT DISABLE_VIDEOSUPPORT)

install(TARGETS kopete_testbed  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_testbed.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
