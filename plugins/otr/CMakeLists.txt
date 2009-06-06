add_subdirectory(icons)

include_directories(${LIBOTR_INCLUDE_DIR} ${KOPETE_INCLUDES})

########### next target ###############

set(kopete_otr_PART_SRCS otrguiclient.cpp otrplugin.cpp)

kde4_add_kcfg_files(kopete_otr_PART_SRCS kopete_otr.kcfgc)

kde4_add_plugin(kopete_otr ${kopete_otr_PART_SRCS})

target_link_libraries(kopete_otr ${KDE4_KDEUI_LIBS} kopete kopete_otr_shared ${LIBOTR_LIBRARY})

install(TARGETS kopete_otr DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_kopete_otr_PART_SRCS otrpreferences.cpp)

kde4_add_ui_files(kcm_kopete_otr_PART_SRCS otrprefs.ui)

kde4_add_kcfg_files(kcm_kopete_otr_PART_SRCS ${kotr_PART_SRCS} kopete_otr.kcfgc)

kde4_add_plugin(kcm_kopete_otr ${kcm_kopete_otr_PART_SRCS})

target_link_libraries(kcm_kopete_otr ${KDE4_KDEUI_LIBS} kopete kopete_otr_shared ${LIBOTR_LIBRARY})

install(TARGETS kcm_kopete_otr DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kopete_otr_shared_LIB_SRCS
    otrlchatinterface.cpp
    otrlconfinterface.cpp
    authenticationwizard.cpp
    privkeypopup.cpp)

kde4_add_ui_files(kopete_otr_shared_LIB_SRCS privkeypopup.ui)

kde4_add_library(kopete_otr_shared SHARED ${kopete_otr_shared_LIB_SRCS})

target_link_libraries(kopete_otr_shared kopete ${KDE4_KIO_LIBS} ${KDE4_KDEUI_LIBS} ${LIBOTR_LIBRARY})

set_target_properties(kopete_otr_shared PROPERTIES VERSION 1.0.0 SOVERSION 1)
install(TARGETS kopete_otr_shared ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############

install(FILES kopete_otr.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kopete_otr.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES otrchatui.rc otrui.rc DESTINATION ${DATA_INSTALL_DIR}/kopete_otr)
install(FILES kopete_otr_config.desktop DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
