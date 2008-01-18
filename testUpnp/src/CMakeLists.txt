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


# Argument test unit
kde4_add_unit_test(argumentTest
	argument.cpp
	./tests/argumenttest.cpp
)

# Libraries needed for argument tests
target_link_libraries(argumentTest
	${KDE4_KIO_LIBS}
	${QT_QTTEST_LIBRARY}
)

# Action test unit
kde4_add_unit_test(actionTest
	action.cpp
	argument.cpp
	./tests/actiontest.cpp
)

# Libraries needed for action tests
target_link_libraries(actionTest
	${KDE4_KIO_LIBS}
	${QT_QTTEST_LIBRARY}
)

# Service test unit
kde4_add_unit_test(serviceTest
	service.cpp
	action.cpp
	util_Xml.cpp
	argument.cpp
	./tests/servicetest.cpp
)

# Libraries needed for service tests
target_link_libraries(serviceTest
	${KDE4_KIO_LIBS}
	${QT_QTTEST_LIBRARY}
	upnp
)

# Device test unit
kde4_add_unit_test(deviceTest
	service.cpp
	argument.cpp
	util_Xml.cpp
	action.cpp
	device.cpp
	./tests/devicetest.cpp
)

# Libraries needed for tests on devices
target_link_libraries(deviceTest
	${KDE4_KIO_LIBS}
	${QT_QTTEST_LIBRARY}
	upnp
)