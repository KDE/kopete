########### define cmake project ###########################

project(kopete_tutorial)
find_package(KDE4 REQUIRED)
include (KDE4Defaults)
include (MacroLibrary)

########### install the plugins desktop file ###############

install(FILES kopete_tutorialplugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
