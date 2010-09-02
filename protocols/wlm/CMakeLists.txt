project(wlm)

ADD_SUBDIRECTORY(icons)

set(kopete_wlm_include_DIRS
    ${KOPETE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui
    ${LIBMSN_INCLUDE_DIR}
)

set(kopete_wlm_ui_SRCS
   ui/wlmaddcontactpage.cpp
   ui/wlmeditaccountwidget.cpp
)

kde4_add_ui_files(kopete_wlm_ui_SRCS
    ui/wlmaddui.ui
    ui/wlminfo.ui 
    ui/wlmaccountpreferences.ui
    ui/wlmchatsessioninkpopup.ui
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
   wlmchatsessioninkarea.cpp
   wlmchatsessioninkaction.cpp
   wlmtransfermanager.cpp
)

kde4_add_plugin(kopete_wlm ${kopete_wlm_PART_SRCS})

set(kopete_wlm_link_LIBS
    ${KDE4_KDEUI_LIBS} 
    ${KDE4_KUTILS_LIBS}
    ${KDE4_KIO_LIBRARY} 
    ${KDE4_KMIME_LIBS}
    ${QT_QTXML_LIBRARY} 
    ${QT_QTNETWORK_LIBRARY} 
    kopete 
    ${LIBMSN_LIBRARIES}
)

if(BUILD_WLM_MEDIASTREAMER)
    set(kopete_wlm_link_LIBS ${kopete_wlm_link_LIBS} ${MEDIASTREAMER_LIBRARIES} ${LIBORTP_LIBRARY})
    set(kopete_wlm_include_DIRS ${kopete_wlm_include_DIRS} ${MEDIASTREAMER_INCLUDE_DIR})
    add_definitions(-DHAVE_MEDIASTREAMER)
endif(BUILD_WLM_MEDIASTREAMER)

if(HAVE_GIFLIB)
    set(kopete_wlm_link_LIBS ${kopete_wlm_link_LIBS} ${GIF_LIBRARIES})
    set(kopete_wlm_include_DIRS ${kopete_wlm_include_DIRS} ${GIF_INCLUDE_DIR})
    add_definitions(-DHAVE_GIFLIB)
endif(HAVE_GIFLIB)

include_directories(${kopete_wlm_include_DIRS})
target_link_libraries(kopete_wlm ${kopete_wlm_link_LIBS})

install(TARGETS kopete_wlm  DESTINATION ${PLUGIN_INSTALL_DIR})

install(FILES  kopete_wlm.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES  wlmchatui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_wlm)
