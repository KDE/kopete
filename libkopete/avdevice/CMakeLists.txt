
include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_videodevice_LIB_SRCS 
   videodevice.cpp 
   videoinput.cpp 
   videodevicepool.cpp 
   videodevicemodelpool.cpp )

kde4_automoc(${kopete_videodevice_LIB_SRCS})

kde4_add_library(kopete_videodevice SHARED ${kopete_videodevice_LIB_SRCS})

target_link_libraries(kopete_videodevice  ${KDE4_KDECORE_LIBS} )

set_target_properties(kopete_videodevice PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopete_videodevice  DESTINATION ${LIB_INSTALL_DIR})


########### next target ###############


########### install files ###############





