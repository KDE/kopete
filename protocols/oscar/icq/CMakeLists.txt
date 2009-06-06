
#add_subdirectory( ui )

include_directories(
${KOPETE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/ui/
${CMAKE_CURRENT_BINARY_DIR}/ui/
${CMAKE_CURRENT_SOURCE_DIR}/../
${CMAKE_CURRENT_BINARY_DIR}/../
${CMAKE_CURRENT_SOURCE_DIR}/../liboscar/
${CMAKE_CURRENT_SOURCE_DIR}/../liboscar/utils
${CMAKE_CURRENT_SOURCE_DIR}/../liboscar/tasks
)


########### next target ###############

set(kopete_icq_ui_SRCS
ui/icqeditaccountwidget.cpp
ui/icquserinfowidget.cpp
ui/icqauthreplydialog.cpp
ui/icqaddcontactpage.cpp
ui/icqsearchdialog.cpp
ui/icqchangepassworddialog.cpp
ui/xtrazicqstatusdialog.cpp
ui/xtrazicqstatuseditor.cpp
ui/xtrazstatusdelegate.cpp
ui/editorwithicon.cpp
ui/iconcells.cpp
ui/infocombobox.cpp
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
ui/xtrazicqstatusui.ui
ui/xtrazicqstatuseditorui.ui
)

set(kopete_icq_PART_SRCS
   ${kopete_icq_ui_SRCS}
   icqstatusmanager.cpp
   icqaccount.cpp
   icqcontact.cpp
   aimcontact.cpp
   icqprotocol.cpp
   xtrazstatus.cpp
   xtrazstatusmodel.cpp
   xtrazstatusaction.cpp )

kde4_add_plugin(kopete_icq ${kopete_icq_PART_SRCS})


target_link_libraries(kopete_icq
    kopete_oscar
    oscar
    kopete
    ${KDE4_KDEUI_LIBS}
    ${QT_QTNETWORK_LIBRARY}
    )

install(TARGETS kopete_icq  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_icq.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




