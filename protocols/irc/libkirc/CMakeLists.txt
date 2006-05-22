set(libkirc_SRCS 
kircengine.cpp 
kircengine_commands.cpp 
kircengine_ctcp.cpp 
kircengine_numericreplies.cpp 
kircentity.cpp 
kircmessage.cpp 
kircmessageredirector.cpp 
kirctransfer.cpp 
kirctransferhandler.cpp 
kirctransferserver.cpp 
ksslsocket.cpp 
)

kde4_automoc(${libkirc_SRCS})

kde4_add_library(kirc SHARED ${libkirc_SRCS})

target_link_libraries(kirc ${KDE4_KIO_LIBS})

install_targets(${LIB_INSTALL_DIR} kirc)
