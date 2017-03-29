
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

ki18n_wrap_ui(kcm_kopete_chatwindowconfig_PART_SRCS 
    chatwindowconfig_style.ui
    chatwindowconfig_emoticons.ui
    chatwindowconfig_colors.ui
    chatwindowconfig_tab.ui )

add_library(kcm_kopete_chatwindowconfig MODULE ${kcm_kopete_chatwindowconfig_PART_SRCS})


target_link_libraries(kcm_kopete_chatwindowconfig  kopete kopetechatwindow_shared ${KDE4_KNEWSTUFF2_LIBS} KF5::KHtml)

install(TARGETS kcm_kopete_chatwindowconfig DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_chatwindowconfig.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})


