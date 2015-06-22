include_directories(${KOPETE_INCLUDES} ${QCA2_LIBRARIES})

#set(kopete_cryptography2_PART_SRCS
#    cryptographygui.cpp)

#kde4_add_ui_files(kopete_cryptography_PART_SRCS maingui.ui)

#kde4_add_plugin(kopete_cryptography2 ${kopete_cryptography2_PART_SRCS})

#target_link_libraries(kopete_cryptography2 ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS})

#install( TARGETS kopete_cryptography2 DESTINATION ${PLUGIN_INSTALL_DIR})

set(kcm_kopete_gnupg_PART_SRCS gnupgpreferences.cpp)

#kde4_add_kcfg_files(kcm_kopete_cryptography2 ${kcm_kopete_cryptography2_PART_SRCS} cryptography2settings.kcfgc)


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

install( FILES kopete_gnupg.desktop  DESTINATION ${SERVICES_INSTALL_DIR})

install( FILES kopete_gnupg_config2.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
