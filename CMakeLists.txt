
project(kopete)
add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config-kopete.h.cmake ${CMAKE_CURRENT_BINARY_DIR}/config-kopete.h )

# For config-kopete.h
include_directories( ${CMAKE_CURRENT_BINARY_DIR} )

set(KOPETE_INCLUDES 
	${KDE4_INCLUDES}
	${CMAKE_SOURCE_DIR}/kopete/libkopete/ 
	${CMAKE_BINARY_DIR}/kopete/libkopete/ 
	${CMAKE_SOURCE_DIR}/kopete/libkopete/ui/ 
	${CMAKE_BINARY_DIR}/kopete/libkopete/ui/ 
	${CMAKE_SOURCE_DIR}/kopete/libkopete/private 
	${CMAKE_SOURCE_DIR}/kopete/libkopete/contactlist/ 
)

add_subdirectory( libkopete ) 
add_subdirectory( kopete ) 
add_subdirectory( protocols ) 
add_subdirectory( plugins ) 
add_subdirectory( icons ) 
add_subdirectory( sounds ) 
add_subdirectory( styles ) 

