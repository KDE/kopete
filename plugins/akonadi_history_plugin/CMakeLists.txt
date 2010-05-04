

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include(MacroLibrary)
include(CheckIncludeFiles)
include(MacroBoolTo01)
find_package(KdepimLibs 4.3.85 REQUIRED)


############### Find the stuff we need ###############

  find_package(Akonadi QUIET NO_MODULE)
  macro_log_feature(Akonadi_FOUND "Akonadi" "Akonadi server libraries" "http://pim.kde.org/akonadi" TRUE "" "Akonadi is required to build KDEPIM")

  find_package(Strigi)
  macro_log_feature(STRIGI_FOUND "Strigi" "Index metadata of files" "http://strigi.sourceforge.net" TRUE "" "")

  find_package(SharedDesktopOntologies ${SDO_MIN_VERSION})
  macro_log_feature(SHAREDDESKTOPONTOLOGIES_FOUND "Shared desktop ontologies" "Desktop ontologies" "http://oscaf.sourceforge.net" TRUE "${SDO_MIN_VERSION}" "Ontologies necessary for the Nepomuk semantic desktop.")

  find_package(Soprano)
  macro_log_feature(Soprano_FOUND "Soprano" "Semantic Desktop Storing" "http://soprano.sourceforge.net" TRUE "${SOPRANO_MIN_VERSION}" "Soprano is needed for Nepomuk")
  include(SopranoAddOntology)

  find_package(Nepomuk)
  macro_log_feature(Nepomuk_FOUND "Nepomuk" "The Nepomuk libraries" "http://www.kde.org" TRUE "" "Nepomuk extends the search and tagging functionalities in KMail and Akonadi")



############### Needed commands before building anything ###############

add_definitions (${QT_DEFINITIONS} ${KDE4_DEFINITIONS} ${KDEPIM_DEFINITIONS})

include_directories (${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR} ${KDEPIMLIBS_INCLUDE_DIRS} ${KDE4_INCLUDES} ${QT_QTDBUS_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/libkdepim)

###################################
add_subdirectory( ontologies )
include_directories( 
    ${KOPETE_INCLUDES} 
    ${KDEPIMLIBS_INCLUDE_DIR}
    ${NEPOMUK_INCLUDES}
    ${SOPRANO_INCLUDE_DIR} 
)
add_definitions( ${KDE4_ENABLE_EXCEPTIONS} )


########### next target ###############

set(akonadi_kopete_history_PART_SRCS  
    akonadihistoryplugin.cpp
    akonadihistorylogger.cpp
    historyactionmanager.cpp
    searchdialog.cpp
)

#TODO:The UI file for search and view of chat history
kde4_add_ui_files(akonadi_kopete_history_PART_SRCS searchdialog.ui )

#TODO:The config files, to de done later
#kde4_add_kcfg_files(akonadi_kopete_history_PART_SRCS historyconfig.kcfgc )


kde4_add_plugin(akonadi_kopete_history ${akonadi_kopete_history_PART_SRCS})


target_link_libraries(
    akonadi_kopete_history  
    ${KDE4_KHTML_LIBS} 
    ${KDE4_AKONADI_LIBS} 
    ${KDE4_AKONADI_KMIME_LIBS} 
    kopete 
    history 
    ${SOPRANO_LIBRARIES}
    niefast_apps
    ${QT_QTWEBKIT_LIBRARY}
    ${NEPOMUK_LIBRARIES}
    ${NEPOMUK_QUERY_LIBRARIES}
)

install(TARGETS akonadi_kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

#TODO:History preference is something that we do not need at the moment
#set(kcm_akonadi_kopete_history_PART_SRCS historypreferences.cpp )
#kde4_add_ui_files(kcm_akonadi_kopete_history_PART_SRCS historyprefsui.ui )
#kde4_add_kcfg_files(kcm_akonadi_kopete_history_PART_SRCS historyconfig.kcfgc )


#kde4_add_plugin(kcm_akonadi_kopete_history ${kcm_akonadi_kopete_history_PART_SRCS})


#target_link_libraries(kcm_akonadi_kopete_history  ${KDE4_KUTILS_LIBS} kopete )

#install(TARGETS kcm_akonadi_kopete_history  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

#install( FILES historyconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES akonadi_kopete_history.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES historyui.rc tagchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/akonadi_kopete_history)
install( FILES akonadi_kopete_history_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)

