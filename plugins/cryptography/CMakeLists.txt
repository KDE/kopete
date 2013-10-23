include_directories (${KLEOPATRA_INCLUDE_DIR} ${KOPETE_INCLUDES})

########### next target ###############

set(kopete_cryptography_PART_SRCS 
   cryptographyplugin.cpp 
   cryptographyguiclient.cpp 
   cryptographyselectuserkey.cpp 
   exportkeys.cpp
   cryptographymessagehandler.cpp )

kde4_add_kcfg_files(kopete_cryptography_PART_SRCS
  cryptographysettings.kcfgc
)

kde4_add_ui_files(kopete_cryptography_PART_SRCS kabckeyselectorbase.ui exportkeysbase.ui )

kde4_add_plugin(kopete_cryptography ${kopete_cryptography_PART_SRCS})

target_link_libraries(kopete_cryptography ${QT_QTGUI_LIBRARY}  ${KDE4_KDECORE_LIBS} kopete ${KLEOPATRA_LIBRARIES} ${KDE4_KABC_LIBS} ${QGPGME_LIBRARIES} )

install(TARGETS kopete_cryptography  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_cryptography_PART_SRCS cryptographypreferences.cpp )

kde4_add_kcfg_files(kcm_kopete_cryptography_PART_SRCS
  cryptographysettings.kcfgc
)

kde4_add_plugin(kcm_kopete_cryptography ${kcm_kopete_cryptography_PART_SRCS})

target_link_libraries(
    kcm_kopete_cryptography
    kopete
    ${KLEOPATRA_LIBRARIES}
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

install(TARGETS kcm_kopete_cryptography  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_cryptography.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES cryptographyui.rc cryptographychatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_cryptography)
install( FILES kopete_cryptography_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
