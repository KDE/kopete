add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_autoreplace_PART_SRCS autoreplaceplugin.cpp autoreplaceconfig.cpp )

add_library(kopete_autoreplace MODULE ${kopete_autoreplace_PART_SRCS})


target_link_libraries(kopete_autoreplace   kopete )

install(TARGETS kopete_autoreplace  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kcm_kopete_autoreplace_PART_SRCS autoreplacepreferences.cpp autoreplaceconfig.cpp )

ki18n_wrap_ui(kcm_kopete_autoreplace_PART_SRCS autoreplaceprefs.ui )


add_library(kcm_kopete_autoreplace MODULE ${kcm_kopete_autoreplace_PART_SRCS})


target_link_libraries(kcm_kopete_autoreplace   kopete )

install(TARGETS kcm_kopete_autoreplace  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_autoreplace.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES kopete_autoreplace_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)




