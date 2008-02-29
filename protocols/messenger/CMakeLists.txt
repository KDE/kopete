
set(PAPILLON_INSIDE_KOPETE TRUE)

add_subdirectory( libpapillon )
add_subdirectory( ui )

include_directories(
${KOPETE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/ui/
${CMAKE_CURRENT_SOURCE_DIR}/libpapillon/)


########### next target ###############

set(kopete_messenger_ui_SRCS
   ui/messengereditaccountwidget.cpp
   ui/messengeraddcontactpage.cpp
)

kde4_add_ui_files(kopete_messenger_ui_SRCS
   ui/messengeradd.ui 
   ui/messengerdebugrawcommand_base.ui 
   ui/messengerinfo.ui 
   ui/messengereditaccount.ui
)

set(kopete_messenger_PART_SRCS 
   messengerprotocol.cpp 
   messengeraccount.cpp 
   messengercontact.cpp 
   ${kopete_messenger_ui_SRCS} 
)

kde4_add_plugin(kopete_messenger ${kopete_messenger_PART_SRCS})

target_link_libraries(kopete_messenger ${KDE4_KDE3SUPPORT_LIBS} ${KDE4_KDECORE_LIBS} kopete)

install(TARGETS kopete_messenger  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_messenger.desktop DESTINATION ${SERVICES_INSTALL_DIR})




