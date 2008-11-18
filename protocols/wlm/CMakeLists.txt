project(wlm)

ADD_SUBDIRECTORY(icons)

include_directories(
    ${KOPETE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
)

set(kopete_wlm_ui_SRCS
   ui/wlmaddcontactpage.cpp
   ui/wlmeditaccountwidget.cpp
)

kde4_add_ui_files(kopete_wlm_ui_SRCS
    ui/wlmaddui.ui
    ui/wlmaccountpreferences.ui
)

set(kopete_wlm_PART_SRCS
   ${kopete_wlm_ui_SRCS}
   wlmprotocol.cpp
   wlmcontact.cpp
   wlmaccount.cpp
   wlmlibmsn.cpp
   wlmsocket.cpp
   wlmserver.cpp
   wlmchatmanager.cpp
   wlmchatsession.cpp
   wlmtransfermanager.cpp
)

kde4_add_plugin(kopete_wlm ${kopete_wlm_PART_SRCS})

target_link_libraries(
    kopete_wlm 
    ${KDE4_KDEUI_LIBS} 
    ${KDE4_KUTILS_LIBS}
    ${KDE4_KIO_LIBRARY} 
    ${QT_QTXML_LIBRARY} 
    ${QT_QTNETWORK_LIBRARY} 
    kopete 
    kopete_videodevice 
    ${LIBMSN_LIBRARIES}
)

install(TARGETS kopete_wlm  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES  kopete_wlm.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  wlmchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_wlm)
