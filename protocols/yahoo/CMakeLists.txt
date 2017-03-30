
add_subdirectory( libkyahoo ) 
#add_subdirectory( ui ) 
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui 
${CMAKE_CURRENT_SOURCE_DIR}/libkyahoo 
${CMAKE_CURRENT_BINARY_DIR}/
${JASPER_INCLUDE_DIR}
)

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/libkyahoo )

########### next target ###############

set(kopete_yahoo_ui_SRCS 
ui/yahooinvitelistimpl.cpp 
ui/yahoowebcamdialog.cpp 
ui/yahoouserinfodialog.cpp 
ui/yahoochatselectordialog.cpp
)

ki18n_wrap_ui(kopete_yahoo_ui_SRCS 
ui/yahooadd.ui 
ui/yahooinvitelistbase.ui 
ui/yahooverifyaccountbase.ui 
ui/yahoostealthsetting.ui 
ui/yahoogeneralinfowidget.ui 
ui/yahooworkinfowidget.ui 
ui/yahoootherinfowidget.ui 
ui/yahooeditaccountbase.ui
ui/yahoochatselectorwidgetbase.ui
)

set(kopete_yahoo_PART_SRCS 
   ${kopete_yahoo_ui_SRCS} 
   yahooprotocol.cpp 
   yahoocontact.cpp 
   yahooaddcontact.cpp 
   yahooaccount.cpp 
   yahooeditaccount.cpp 
   yahooconferencemessagemanager.cpp 
   yahoochatchatsession.cpp
   yahoochatsession.cpp 
   yahooverifyaccount.cpp 
   yahoowebcam.cpp )

qt5_add_resources(kopete_yahoo_PART_SRCS yahooprotocol.qrc)   
   
ecm_qt_declare_logging_category(kopete_yahoo_PART_SRCS HEADER yahoo_protocol_debug.h IDENTIFIER YAHOO_PROTOCOL_LOG CATEGORY_NAME org.kde.kopete.yahoo)

add_library(kopete_yahoo MODULE ${kopete_yahoo_PART_SRCS})

target_link_libraries(kopete_yahoo KF5::KIOCore kopete kyahoo ${QT_QT3SUPPORT_LIBS} ${JASPER_LIBRARIES})
if(NOT DISABLE_VIDEOSUPPORT)
  target_link_libraries(kopete_yahoo kopete_videodevice)
endif(NOT DISABLE_VIDEOSUPPORT)

install(TARGETS kopete_yahoo  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_yahoo.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})




