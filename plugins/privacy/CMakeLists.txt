
#add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)

#common lib
set(kopete_privacy_LIB_SRCS privacymessagehandler.cpp privacyaccountlistmodel.cpp)
kconfig_add_kcfg_files(kopete_privacy_LIB_SRCS privacyconfig.kcfgc )
add_library( kopeteprivacy SHARED ${kopete_privacy_LIB_SRCS})
target_link_libraries( kopeteprivacy  kopete )
set_target_properties( kopeteprivacy PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install( TARGETS kopeteprivacy ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(kopete_privacy_PART_SRCS privacyplugin.cpp privacyguiclient.cpp)
add_library(kopete_privacy MODULE ${kopete_privacy_PART_SRCS})
target_link_libraries(kopete_privacy   kopete kopeteprivacy )
install(TARGETS kopete_privacy  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kcm_kopete_privacy_PART_SRCS privacypreferences.cpp contactselectorwidget.cpp )
ki18n_wrap_ui(kcm_kopete_privacy_PART_SRCS privacydialog.ui contactselectorwidget_base.ui )
add_library(kcm_kopete_privacy MODULE ${kcm_kopete_privacy_PART_SRCS})
target_link_libraries(kcm_kopete_privacy kopete kopeteprivacy )
install(TARGETS kcm_kopete_privacy  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES privacyui.rc  DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kopete)
install( FILES kopete_privacy.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES kopete_privacy_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)
