# Project name
project(testsProgress)

# Requirements
find_package(KDE4 REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES})
include_directories(/usr/local/include)

# Project's sources
set(PROGRESS_SRCS 
	main.cpp
	ProgressBarSample.cpp
	)

# Linking the GUI file
kde4_add_ui_files(PROGRESS_SRCS progressTests.ui)

# Build executable
kde4_add_executable(progress ${PROGRESS_SRCS})

# Required libraries
target_link_libraries(progress ${KDE4_KIO_LIBS} )

# Ready to install
install(FILES
	ProgressBarSample.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kopete/upnp/sample_progress_bar)