# Project name
project(solt)

find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})

# Project sources
set(projectSources 
	test.cpp)

# Build executable
kde4_add_executable(solt ${projectSources})

# Libraries needed by the application
target_link_libraries(solt ${KDE4_KIO_LIBS})
