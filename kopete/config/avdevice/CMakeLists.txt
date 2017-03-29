
include_directories(
    ${KOPETE_INCLUDES}

    )

########### next target ###############

set(kcm_kopete_avdeviceconfig_PART_SRCS
    avdeviceconfig.cpp IdGuiElements.cpp )
# avdeviceconfig_videodevicec.cpp )

ki18n_wrap_ui(kcm_kopete_avdeviceconfig_PART_SRCS avdeviceconfig_videodevice.ui )

add_library(kcm_kopete_avdeviceconfig MODULE ${kcm_kopete_avdeviceconfig_PART_SRCS})


target_link_libraries(kcm_kopete_avdeviceconfig   kopete kopete_videodevice)

install(TARGETS kcm_kopete_avdeviceconfig  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_avdeviceconfig.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

#ecm_install_icons( ${KDE_INSTALL_DATADIR}/kopete/icons  )
ecm_install_icons(${KDE_INSTALL_ICONDIR})
