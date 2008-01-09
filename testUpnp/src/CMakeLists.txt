# Project name
project(window)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})
include_directories(/usr/local/include) 

# Project sources
set(projectSources 
	main.cpp
	applicationWidget.cpp
	upnpKopete.cpp
	action.cpp
	argument.cpp
	service.cpp
	device.cpp
	util_Xml.cpp)

kde4_add_ui_files(projectSources mainWindow.ui)

# Build executable
kde4_add_executable(window ${projectSources})

# Libraries needed by the application
target_link_libraries(window ${KDE4_KIO_LIBS} upnp )
