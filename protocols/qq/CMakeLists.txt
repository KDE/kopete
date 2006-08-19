
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
)


########### next target ###############

set(kopete_qq_PART_SRCS 
   qqprotocol.cpp 
   qqcontact.cpp 
   qqaccount.cpp 
   qqaddcontactpage.cpp 
   qqeditaccountwidget.cpp 
   qqsocket.cpp
   qqnotifysocket.cpp
   qqchatsession.cpp
   ui/qqwebcamdialog.cpp 
   libeva.cpp
   evautil.cpp
   md5.c
   crypt.cpp
)

kde4_automoc(${kopete_qq_PART_SRCS})

kde4_add_ui_files(kopete_qq_PART_SRCS
qqeditaccountui.ui
)

kde4_add_ui_files(kopete_qq_PART_SRCS
qqaddui.ui
)


kde4_add_plugin(kopete_qq ${kopete_qq_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_qq)

target_link_libraries(kopete_qq ${KDE4_KIO_LIBS} ${QT_QTNETWORK_LIBRARY} kopete kopete_videodevice )

install(TARGETS kopete_qq DESTINATION ${PLUGIN_INSTALL_DIR} )


########### install files ###############

install(FILES kopete_qq.desktop DESTINATION ${SERVICES_INSTALL_DIR} )

