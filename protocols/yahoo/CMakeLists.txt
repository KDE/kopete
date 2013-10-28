
add_subdirectory( libkyahoo ) 
#add_subdirectory( ui ) 
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui 
${CMAKE_CURRENT_SOURCE_DIR}/libkyahoo 
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

kde4_add_ui_files(kopete_yahoo_ui_SRCS 
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


kde4_add_plugin(kopete_yahoo ${kopete_yahoo_PART_SRCS})

target_link_libraries(kopete_yahoo ${KDE4_KIO_LIBS} kopete kyahoo ${QT_QT3SUPPORT_LIBS} ${JASPER_LIBRARIES})
if(NOT DISABLE_VIDEOSUPPORT)
  target_link_libraries(kopete_yahoo kopete_videodevice)
endif(NOT DISABLE_VIDEOSUPPORT)

install(TARGETS kopete_yahoo  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_yahoo.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES yahooconferenceui.rc yahooimui.rc yahoochatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_yahoo)




