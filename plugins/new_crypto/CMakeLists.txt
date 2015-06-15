include_directories (${KLEOPATRA_INCLUDE_DIR} ${KOPETE_INCLUDES})

########### next target ###############

set(kopete_new_crypto_PART_SRCS 
   cryptoplugin.cpp 
   cryptoclient.cpp 
   cryptoselectuserkey.cpp 
   cryptoexportkeys.cpp
   cryptomessagehandler.cpp )

kde4_add_kcfg_files(kopete_new_crypto_PART_SRCS
  cryptosettings.kcfgc
)

kde4_add_ui_files(kopete_new_crypto_PART_SRCS kabckeyselectorbase.ui cryptoexportkeysbase.ui )

kde4_add_plugin(kopete_new_crypto ${kopete_new_crypto_PART_SRCS})

target_link_libraries(kopete_new_crypto ${QT_QTGUI_LIBRARY}  ${KDE4_KDECORE_LIBS} kopete ${KDE4_KABC_LIBS} ${QGPGME_LIBRARIES} )

install(TARGETS kopete_new_crypto  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_new_crypto_PART_SRCS cryptopreferences.cpp )

kde4_add_kcfg_files(kcm_kopete_cryptography_PART_SRCS
  cryptosettings.kcfgc
)

kde4_add_plugin(kcm_kopete_new_crypto ${kcm_new_kopete_crypto_PART_SRCS})

target_link_libraries(
    kcm_kopete_new_crypto
    kopete
    ${KDE4_KDECORE_LIBS}
    ${KDE4_KDEUI_LIBS}
    ${KDE4_KUTILS_LIBS}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTGUI_LIBRARY}
    )

install(TARGETS kcm_kopete_new_crypto  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_new_crypto.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES new_cryptoui.rc new_cryptochatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_new_crypto)
install( FILES kopete_new_crypto_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)

