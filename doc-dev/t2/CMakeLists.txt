########### define cmake project ###########################

project(kopete_tutorial)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

########### install the plugin desktop file ###############

install(FILES kopete_tutorialplugin.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})

########### library containing plugin logic ###############

# set the search path for included headers
set(KOPETE_INCLUDES
    /kopete
    
   )

include_directories(${KOPETE_INCLUDES})

# the basic list of sources
set(kopete_tutorialplugin_PART_SRCS tutorialplugin.cpp )

# create moc files as needed

# declare the plugin library
add_library(kopete_tutorialplugin MODULE ${kopete_tutorialplugin_PART_SRCS})

# link any other libraries
# kdeui is needed because a kopete plugin uses KXMLGUI
target_link_libraries(kopete_tutorialplugin  kopete )

# install the result with other KDE plugins
install(TARGETS kopete_tutorialplugin DESTINATION ${KDE_INSTALL_PLUGINDIR})


