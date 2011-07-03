
include_directories( 
${KOPETE_INCLUDES} 
${KOPETE_SOURCE_DIR}/kopete/chatwindow
${KOPETE_BINARY_DIR}/kopete/chatwindow
)


########### next target ###############

set(kcm_kopete_chatwindowconfig_PART_SRCS
   chatwindowconfig.cpp  
   emoticonthemedelegate.cpp 
   emoticonthemeitem.cpp )

kde4_add_ui_files(kcm_kopete_chatwindowconfig_PART_SRCS 
   chatwindowconfig_style.ui
   chatwindowconfig_emoticons.ui
   chatwindowconfig_colors.ui
   chatwindowconfig_tab.ui )

kde4_add_plugin(kcm_kopete_chatwindowconfig ${kcm_kopete_chatwindowconfig_PART_SRCS})


target_link_libraries(kcm_kopete_chatwindowconfig ${KDE4_KUTILS_LIBS} kopete kopetechatwindow_shared ${KDE4_KNEWSTUFF2_LIBS} ${KDE4_KHTML_LIBS})

install(TARGETS kcm_kopete_chatwindowconfig DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_chatwindowconfig.desktop  DESTINATION ${SERVICES_INSTALL_DIR})


