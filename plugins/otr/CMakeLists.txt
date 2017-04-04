add_subdirectory(icons)

include_directories(${LIBOTR_INCLUDE_DIR} ${KOPETE_INCLUDES})

########### next target ###############

set(kopete_otr_common_SRCS)
ecm_qt_declare_logging_category(kopete_otr_common_SRCS HEADER plugin_otr_debug.h IDENTIFIER KOPETE_PLUGIN_OTR_LOG CATEGORY_NAME org.kde.kopete.plugin.otr)

set(kopete_otr_PART_SRCS otrguiclient.cpp otrplugin.cpp ${kopete_otr_common_SRCS})

kconfig_add_kcfg_files(kopete_otr_PART_SRCS kopete_otr.kcfgc)

add_library(kopete_otr MODULE ${kopete_otr_PART_SRCS})

target_link_libraries(kopete_otr  kopete kopete_otr_shared ${LIBOTR_LIBRARY})

install(TARGETS kopete_otr DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kcm_kopete_otr_PART_SRCS otrpreferences.cpp)

ki18n_wrap_ui(kcm_kopete_otr_PART_SRCS otrprefs.ui)

kconfig_add_kcfg_files(kcm_kopete_otr_PART_SRCS ${kotr_PART_SRCS} kopete_otr.kcfgc)

add_library(kcm_kopete_otr MODULE ${kcm_kopete_otr_PART_SRCS})

target_link_libraries(kcm_kopete_otr  kopete kopete_otr_shared ${LIBOTR_LIBRARY})

install(TARGETS kcm_kopete_otr DESTINATION ${KDE_INSTALL_PLUGINDIR})

########### next target ###############

set(kopete_otr_shared_LIB_SRCS
    ${kopete_otr_common_SRCS}
    otrlchatinterface.cpp
    otrlconfinterface.cpp
    authenticationwizard.cpp
    privkeypopup.cpp)

ki18n_wrap_ui(kopete_otr_shared_LIB_SRCS privkeypopup.ui)

add_library(kopete_otr_shared SHARED ${kopete_otr_shared_LIB_SRCS})

target_link_libraries(kopete_otr_shared kopete KF5::KIOCore  ${LIBOTR_LIBRARY})

set_target_properties(kopete_otr_shared PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS kopete_otr_shared ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES kopete_otr.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kopete_otr.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES otrchatui.rc otrui.rc DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kopete_otr)
install(FILES kopete_otr_config.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)
