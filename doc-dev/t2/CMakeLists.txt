########### define cmake project ###########################

project(kopete_tutorial)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

########### install the plugin desktop file ###############

install(FILES kopete_tutorialplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})

########### library containing plugin logic ###############

# set the search path for included headers
set(KOPETE_INCLUDES
    ${KDE4_INCLUDE_DIR}/kopete
    ${KDE4_INCLUDES}
   )

include_directories(${KOPETE_INCLUDES})

# the basic list of sources
set(kopete_tutorialplugin_PART_SRCS tutorialplugin.cpp )

# create moc files as needed

# declare the plugin library
kde4_add_plugin(kopete_tutorialplugin ${kopete_tutorialplugin_PART_SRCS})

# link any other libraries
# kdeui is needed because a kopete plugin uses KXMLGUI
target_link_libraries(kopete_tutorialplugin ${KDE4_KDEUI_LIBS} kopete )

# install the result with other KDE plugins
install(TARGETS kopete_tutorialplugin DESTINATION ${PLUGIN_INSTALL_DIR})


