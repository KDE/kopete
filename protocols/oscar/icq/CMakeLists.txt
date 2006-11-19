
#add_subdirectory( ui ) 

include_directories(
${KOPETE_INCLUDES} 
${CMAKE_CURRENT_SOURCE_DIR}/ui/ 
${CMAKE_CURRENT_BINARY_DIR}/ui/ 
${CMAKE_CURRENT_SOURCE_DIR}/../ 
${CMAKE_CURRENT_BINARY_DIR}/../ 
${CMAKE_CURRENT_SOURCE_DIR}/../liboscar/ 
)


########### next target ###############

set(kopete_icq_ui_SRCS 
ui/icqeditaccountwidget.cpp 
ui/icquserinfowidget.cpp 
ui/icqauthreplydialog.cpp 
ui/icqaddcontactpage.cpp 
ui/icqsearchdialog.cpp
ui/icqchangepassworddialog.cpp
)

kde4_add_ui_files(kopete_icq_ui_SRCS 
ui/icqadd.ui 
ui/icqgeneralinfo.ui 
ui/icqotherinfowidget.ui 
ui/icqhomeinfowidget.ui 
ui/icqworkinfowidget.ui 
ui/icqinterestinfowidget.ui 
ui/icqorgaffinfowidget.ui 
ui/icqauthreplyui.ui 
ui/icqsearchbase.ui 
ui/icqeditaccountui.ui
ui/icqchangepassword.ui
)

set(kopete_icq_PART_SRCS 
   ${kopete_icq_ui_SRCS} 
   icqpresence.cpp 
   icqaccount.cpp 
   icqcontact.cpp 
   aimcontact.cpp 
   icqprotocol.cpp )

kde4_automoc(${kopete_icq_PART_SRCS})

kde4_add_plugin(kopete_icq ${kopete_icq_PART_SRCS})


target_link_libraries(kopete_icq ${KDE4_KDECORE_LIBS} kopete_oscar kopete )

install(TARGETS kopete_icq  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_icq.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES x-icq.desktop  DESTINATION ${MIME_INSTALL_DIR}/application)




