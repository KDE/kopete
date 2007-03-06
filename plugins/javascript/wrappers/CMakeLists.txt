
INCLUDE_DIRECTORIES(
    ${KOPETE_INCLUDES}
)


########### next target ###############

set(kopetemessageplugin_PART_SRCS
    kopetemessageimp.cpp
)

kde4_automoc(${kopetemessageplugin_PART_SRCS})

kde4_add_plugin(kopetemessageplugin ${kopetemessageplugin_PART_SRCS})

target_link_libraries(kopetemessageplugin  ${KDE4_KDECORE_LIBS} ${KDE4_KPARTS_LIBS} kjsembed kopete )

install(TARGETS kopetemessageplugin DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(wrappers_STAT_SRCS
    kopeteonlinestatusimp.cpp
)

kde4_automoc(${wrappers_STAT_SRCS})

KDE4_ADD_LIBRARY(wrappers STATIC ${wrappers_STAT_SRCS})


########### install files ###############

install( FILES kopetemessageplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR} )



