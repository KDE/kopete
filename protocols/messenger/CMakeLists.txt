set(PAPILLON_INSIDE_KOPETE TRUE)

add_subdirectory( libpapillon ) 

include_directories( 
${KOPETE_INCLUDES} 
)


########### next target ###############

set(kopete_messenger_PART_SRCS messengerprotocol.cpp messengeraccount.cpp messengercontact.cpp )


kde4_add_plugin(kopete_messenger ${kopete_messenger_PART_SRCS})


target_link_libraries(kopete_messenger  ${KDE4_KIO_LIBS} kopete )

install(TARGETS kopete_messenger  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_messenger.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




