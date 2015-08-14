include_directories(${KOPETE_INCLUDES} ${QCA2_LIBRARIES})

## Main Plugin ##

set(kopete_gnupg_PART_SRCS
    gnupgplugin.cpp
    gnupgselectuserkey.cpp)

#kde4_add_ui_files(kopete_cryptography_PART_SRCS gnupggui.ui)

kde4_add_kcfg_files(kopete_gnupg_PART_SRCS gnupgsettings.kcfgc )

kde4_add_plugin(kopete_gnupg ${kopete_gnupg_PART_SRCS})

target_link_libraries(kopete_gnupg ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} kopete ${KDE4_KDEUI_LIBS} ${QCA2_LIBRARIES})
install(TARGETS kopete_gnupg DESTINATION ${PLUGIN_INSTALL_DIR})

## KCM ##

set(kcm_kopete_gnupg_PART_SRCS gnupgpreferences.cpp)

#kde4_add_kcfg_files(kcm_kopete_gnupg ${kcm_kopete_cryptography2_PART_SRCS} gnupgsettings.kcfgc)
kde4_add_kcfg_files(kcm_kopete_gnupg_PART_SRCS gnupgsettings.kcfgc)

kde4_add_plugin(kcm_kopete_gnupg ${kcm_kopete_gnupg_PART_SRCS})


target_link_libraries(
kcm_kopete_gnupg
kopete
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    ${QCA2_LIBRARIES}
)

install( TARGETS kcm_kopete_gnupg DESTINATION ${PLUGIN_INSTALL_DIR})

install( FILES gnupgsettings.kcfg  DESTINATION ${KCFG_INSTALL_DIR})

install( FILES gnupgui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_gnupg)

install( FILES kopete_gnupg.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

install( FILES kopete_gnupg_config2.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
