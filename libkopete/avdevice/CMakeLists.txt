
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_videodevice_LIB_SRCS 
   videodevice.cpp 
   videoinput.cpp 
   videodevicepool.cpp 
   videodevicemodelpool.cpp )

kde4_add_library(kopete_videodevice SHARED ${kopete_videodevice_LIB_SRCS})

target_link_libraries(kopete_videodevice ${QT_QTGUI_LIBRARY} ${KDE4_KDECORE_LIBS} )

set_target_properties(kopete_videodevice PROPERTIES VERSION ${GENERIC_LIB_VERSION} SOVERSION ${GENERIC_LIB_SOVERSION} )
install(TARGETS kopete_videodevice  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############


########### install files ###############





