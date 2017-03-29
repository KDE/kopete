
include_directories(
    ${KOPETE_INCLUDES}
    )

########### next target ###############

set(kopete_addbookmarks_PART_SRCS addbookmarksplugin.cpp addbookmarksprefssettings.cpp )


add_library(kopete_addbookmarks MODULE ${kopete_addbookmarks_PART_SRCS})


target_link_libraries(kopete_addbookmarks  KF5::KIOCore kopete )

install(TARGETS kopete_addbookmarks  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kcm_kopete_addbookmarks_PART_SRCS addbookmarkspreferences.cpp addbookmarksprefssettings.cpp )

ki18n_wrap_ui(kcm_kopete_addbookmarks_PART_SRCS addbookmarksprefsui.ui )


add_library(kcm_kopete_addbookmarks MODULE ${kcm_kopete_addbookmarks_PART_SRCS})


target_link_libraries(kcm_kopete_addbookmarks kopete  )

install(TARGETS kcm_kopete_addbookmarks  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_addbookmarks.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES kopete_addbookmarks_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)




