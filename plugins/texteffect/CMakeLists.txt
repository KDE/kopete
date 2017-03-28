
add_subdirectory( icons ) 

include_directories( 
${KOPETE_INCLUDES} 
)

set(kopete_texteffect_common_SRCS)
ecm_qt_declare_logging_category(kopete_texteffect_common_SRCS HEADER plugin_texteffect_debug.h IDENTIFIER KOPETE_PLUGIN_TEXTEFFECT_LOG CATEGORY_NAME org.kde.plugin.texteffect)
########### next target ###############

set(kopete_texteffect_PART_SRCS texteffectplugin.cpp texteffectconfig.cpp ${kopete_texteffect_common_SRCS})

add_library(kopete_texteffect MODULE ${kopete_texteffect_PART_SRCS})


target_link_libraries(kopete_texteffect   kopete )

install(TARGETS kopete_texteffect  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kcm_kopete_texteffect_PART_SRCS texteffectconfig.cpp texteffectpreferences.cpp ${kopete_texteffect_common_SRCS})

ki18n_wrap_ui(kcm_kopete_texteffect_PART_SRCS texteffectprefs.ui )


add_library(kcm_kopete_texteffect MODULE ${kcm_kopete_texteffect_PART_SRCS})


target_link_libraries(kcm_kopete_texteffect   kopete )

install(TARGETS kcm_kopete_texteffect  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_texteffect.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES kopete_texteffect_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)




