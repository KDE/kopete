set(PAPILLON_INSIDE_KOPETE TRUE)

add_subdirectory( libpapillon ) 

include_directories( 
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_BINARY_DIR}/ui/ 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
)


########### next target ###############

set(kopete_messenger_PART_SRCS messengerprotocol.cpp messengeraccount.cpp messengercontact.cpp )

kde4_automoc(${kopete_messenger_PART_SRCS})

kde4_add_plugin(kopete_messenger ${kopete_messenger_PART_SRCS})


target_link_libraries(kopete_messenger  ${KDE4_KIO_LIBS} kopete )

install(TARGETS kopete_messenger  DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kopete_messenger_ui_SRCS ui/messengereditaccountwidget.cpp)

kde4_add_ui_files(kopete_messenger_ui_SRCS
   ui/messengeradd.ui 
   ui/messengerdebugrawcommand_base.ui 
   ui/messengerinfo.ui 
   ui/messengereditaccount.ui
)

set(kopete_messenger_shared_LIB_SRCS 
   ${kopete_messenger_ui_SRCS}
   )

kde4_automoc(${kopete_messenger_shared_LIB_SRCS})

kde4_add_library(kopete_messenger_shared SHARED ${kopete_messenger_shared_LIB_SRCS})

########### install files ###############

install( FILES kopete_messenger.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




