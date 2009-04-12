project ( kopete-cryptography )

set( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake/modules )

find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)
include(MacroOptionalAddSubdirectory)
include (MacroEnsureVersion)
find_package(Kopete REQUIRED)
find_package(KdepimLibs REQUIRED)
find_package(Kleopatra REQUIRED)
find_package(QGpgme REQUIRED)

macro_log_feature(KLEOPATRA_FOUND "Kleopatra" "The Kleopatra libraries (from kdepim)." "" "Kleopatra is required to build the kopete-cryptography plugin.")
find_program( _GPGMECONFIG_EXECUTABLE NAMES gpgme-config )
if ( _GPGMECONFIG_EXECUTABLE )
  message( STATUS "Found gpgme-config at ${_GPGMECONFIG_EXECUTABLE}" )
  exec_program( ${_GPGMECONFIG_EXECUTABLE} ARGS --version OUTPUT_VARIABLE GPGME_VERSION )
  set( _GPGME_MIN_VERSION "1.1.4" )
  macro_ensure_version( ${_GPGME_MIN_VERSION} ${GPGME_VERSION} _GPGME_INSTALLED_VERSION_OK )
  if ( NOT _GPGME_INSTALLED_VERSION_OK )
    message( FATAL_ERROR "The installed version of gpgme is too old: ${GPGME_VERSION} (required: >= ${_GPGME_MIN_VERSION}). Install a newer gpgme, then go back and reconfigure kdepimlibs" )
  endif( NOT _GPGME_INSTALLED_VERSION_OK )
elseif (_GPGMECONFIG_EXECUTABLE)
  message ( FATAL_ERROR "Could not find gpgme-config executable" )
endif( _GPGMECONFIG_EXECUTABLE )

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS})

include_directories (${CMAKE_SOURCE_DIR} 
${KDE4_INCLUDES} ${KDEPIMLIBS_INCLUDE_DIR} ${KLEOPATRA_INCLUDE_DIR})


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
