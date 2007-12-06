project(window)
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})
include_directories(/usr/local/include) 



set(projectSources main.cpp)
set(projectSources ${projectSources} applicationWidget.cpp upnpKopete.cpp sample_util.cpp)

kde4_add_ui_files(projectSources mainWindow.ui)
kde4_add_executable(window ${projectSources})
target_link_libraries(window ${KDE4_KIO_LIBS} upnp )
