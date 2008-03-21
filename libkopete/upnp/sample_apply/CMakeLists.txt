
# Project name
project(apiUPnp)


set(CMAKE_MODULE_PATH /home/kde-devel/kde/src/KDE/kdenetwork/cmake/modules )
find_package(KDE4 REQUIRED)
FIND_PACKAGE(Upnp REQUIRED)
include(KDE4Defaults)
include_directories(${KDE4_INCLUDES} ${UPNP_INCLUDE_DIR})
# include_directories(/usr/local/include) 

# Project sources
set(UPnp_SRCS 
	main.cpp
	applicationWidget.cpp
	../upnpRouter.cpp
	../action.cpp
	../argument.cpp
	../router.cpp
	../service.cpp
	../UPnp.cpp
	../upnpRouterPrivate.cpp
	../util_Xml.cpp
	)

kde4_add_ui_files(UPnp_SRCS mainWindow.ui)

# Build executable
kde4_add_executable(apiUPnp ${UPnp_SRCS})

# Libraries needed by the application
target_link_libraries(apiUPnp ${KDE4_KIO_LIBS} ${UPNP_LIBRARY} ${IXML_LIBRARY} ${THREADUTIL_LIBRARY})

install(FILES
	applicationWidget.h
	../upnpRouter.h
DESTINATION ${INCLUDE_INSTALL_DIR}/kopete/upnp/sample_apply)
