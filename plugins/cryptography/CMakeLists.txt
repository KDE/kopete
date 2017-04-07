include_directories (${KOPETE_INCLUDES})

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

target_link_libraries(kopete_cryptography Qt5::Gui  KF5::KDELibs4Support kopete Gpgmepp )

install(TARGETS kopete_cryptography  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kcm_kopete_cryptography_PART_SRCS cryptographypreferences.cpp )

kconfig_add_kcfg_files(kcm_kopete_cryptography_PART_SRCS
  cryptographysettings.kcfgc
)

add_library(kcm_kopete_cryptography MODULE ${kcm_kopete_cryptography_PART_SRCS})

target_link_libraries(
    kcm_kopete_cryptography
    kopete
    KF5::KDELibs4Support
    
    
    Qt5::Core
    Qt5::Gui
    )

install(TARGETS kcm_kopete_cryptography  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_cryptography.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES cryptographyui.rc cryptographychatui.rc  DESTINATION ${KDE_INSTALL_DATADIR}/kopete_cryptography)
install( FILES kopete_cryptography_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)
