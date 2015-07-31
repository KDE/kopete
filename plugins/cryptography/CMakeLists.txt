include_directories (${KLEOPATRA_INCLUDE_DIR} ${KOPETE_INCLUDES})

########### next target ###############

set(kopete_cryptography_PART_SRCS 
   cryptographyplugin.cpp 
   cryptographyguiclient.cpp 
   cryptographyselectuserkey.cpp 
   exportkeys.cpp
   cryptographymessagehandler.cpp )

kconfig_add_kcfg_files(kopete_cryptography_PART_SRCS
  cryptographysettings.kcfgc
)

ki18n_wrap_ui(kopete_cryptography_PART_SRCS kabckeyselectorbase.ui exportkeysbase.ui )

add_library(kopete_cryptography MODULE ${kopete_cryptography_PART_SRCS})

target_link_libraries(kopete_cryptography Qt5::Gui  KF5::KDELibs4Support kopete ${KLEOPATRA_LIBRARIES} ${KDE4_KABC_LIBS} ${QGPGME_LIBRARIES} )

install(TARGETS kopete_cryptography  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_cryptography_PART_SRCS cryptographypreferences.cpp )

kconfig_add_kcfg_files(kcm_kopete_cryptography_PART_SRCS
  cryptographysettings.kcfgc
)

add_library(kcm_kopete_cryptography MODULE ${kcm_kopete_cryptography_PART_SRCS})

target_link_libraries(
    kcm_kopete_cryptography
    kopete
    ${KLEOPATRA_LIBRARIES}
    KF5::KDELibs4Support
    
    
    Qt5::Core
    Qt5::Gui
    )

install(TARGETS kcm_kopete_cryptography  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_cryptography.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES cryptographyui.rc cryptographychatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_cryptography)
install( FILES kopete_cryptography_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
