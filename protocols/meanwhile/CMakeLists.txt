
add_subdirectory( ui ) 
add_subdirectory( icons ) 




########### next target ###############

set(kopete_meanwhile_PART_SRCS 
   meanwhileprotocol.cpp 
   meanwhileaddcontactpage.cpp 
   meanwhileeditaccountwidget.cpp 
   meanwhileaccount.cpp 
   meanwhilecontact.cpp 
   meanwhilesession.cpp 
   meanwhileplugin.cpp )

kde4_add_plugin(kopete_meanwhile ${kopete_meanwhile_PART_SRCS})


target_link_libraries(kopete_meanwhile  ${KDE4_KDECORE_LIBS} kopete kopetemeanwhileui )

install(TARGETS kopete_meanwhile  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_meanwhile.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




