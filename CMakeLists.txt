cmake_minimum_required(VERSION 2.6)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

find_package(KdepimLibs REQUIRED)

set(KOPETE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(KOPETE_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kopete.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kopete.h )

set(KOPETE_INCLUDES 
	${KDE4_INCLUDES}
	${KOPETE_SOURCE_DIR}/libkopete/
	${KOPETE_BINARY_DIR}/libkopete/
	${KOPETE_SOURCE_DIR}/libkopete/ui/
	${KOPETE_BINARY_DIR}/libkopete/ui/
	${KOPETE_SOURCE_DIR}/libkopete/private/
	${KOPETE_SOURCE_DIR}/libkopete/contactlist/
	${KOPETE_SOURCE_DIR}/libkopete/tasks/
	${KOPETE_BINARY_DIR}
)



add_subdirectory(libkopete)
add_subdirectory(chatwindow)
