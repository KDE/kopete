
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
)


########### next target ###############

set(kopete_qq_ui_SRCS
   ui/qqwebcamdialog.cpp 
   ui/qqeditaccountwidget.cpp 
   ui/dlgqqvcard.cpp 
)
	
ki18n_wrap_ui(kopete_qq_ui_SRCS
   ui/qqeditaccountui.ui
   ui/qqaddui.ui
   ui/qqvcard.ui
)
set(kopete_qq_PART_SRCS 
   ${kopete_qq_ui_SRCS} 
   qqprotocol.cpp 
   qqcontact.cpp 
   qqaccount.cpp 
   qqaddcontactpage.cpp 
   qqsocket.cpp
   qqnotifysocket.cpp
   qqchatsession.cpp
   libeva.cpp
   evautil.cpp
   md5.c
   crypt.cpp
)


add_library(kopete_qq MODULE ${kopete_qq_PART_SRCS})

target_link_libraries(kopete_qq KF5::KIOCore Qt5::Network ${QT_QT3SUPPORT_LIBRARY} kopete)
if(NOT DISABLE_VIDEOSUPPORT)
  target_link_libraries(kopete_qq kopete_videodevice)
endif(NOT DISABLE_VIDEOSUPPORT)

install(TARGETS kopete_qq DESTINATION ${KDE_INSTALL_PLUGINDIR} )


########### install files ###############

install(FILES kopete_qq.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )

