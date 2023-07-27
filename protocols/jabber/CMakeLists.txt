add_definitions(-DIRIS_XMPP_JID_DEPRECATED)

add_subdirectory( icons ) 
add_subdirectory( libiris ) 

#FIXME:glib : necessary ?
include_directories( 
    ${KOPETE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/ui/
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks/
    ${CMAKE_CURRENT_SOURCE_DIR}/libiris/include/iris
    ${CMAKE_CURRENT_SOURCE_DIR}/libiris/src
    )

link_directories( ${CMAKE_CURRENT_BINARY_DIR}/libiris/ )

########### next target ###############

set(kopete_jabber_ui_SRCS 
    ui/dlgxmppconsole.cpp
    ui/jabberaddcontactpage.cpp
    ui/dlgjabbervcard.cpp
    ui/dlgjabberservices.cpp
    ui/dlgregister.cpp
    ui/dlgsearch.cpp
    ui/dlgahcommand.cpp
    ui/dlgahclist.cpp
    ui/jabbereditaccountwidget.cpp
    ui/jabberregisteraccount.cpp
    ui/jabberchooseserver.cpp
    ui/dlgjabberbookmarkeditor.cpp
    ui/dlgjabberchangepassword.cpp
    ui/dlgjabberchatroomslist.cpp
    ui/dlgjabberchatjoin.cpp
    ui/privacylistblockedmodel.cpp
    ui/privacylistmodel.cpp
    ui/privacydlg.cpp
    ui/privacyruledlg.cpp
    ui/dlgjabberxoauth2.cpp
    )

ecm_qt_declare_logging_category(kopete_jabber_ui_SRCS HEADER jabber_protocol_debug.h IDENTIFIER JABBER_PROTOCOL_LOG CATEGORY_NAME org.kde.kopete.jabber)

set(kopete_jabber_ui_files
    #   ${kopete_jabber_ui_SRCS}
    ui/dlgxmppconsole.ui
    ui/dlgjabberregisteraccount.ui
    ui/dlgjabberchooseserver.ui
    ui/dlgvcard.ui
    ui/dlgservices.ui
    ui/dlgaddcontact.ui
    ui/dlgsearch.ui
    ui/dlgchangepassword.ui
    ui/dlgjabberbookmarkeditor.ui
    ui/dlgjabbereditaccountwidget.ui
    ui/dlgchatjoin.ui
    ui/dlgchatroomslist.ui
    ui/privacyrule.ui
    ui/privacy.ui
    ui/dlgxoauth2.ui
    )

ki18n_wrap_ui(kopete_jabber_ui_SRCS ${kopete_jabber_ui_files})

set(kopete_jabber_PART_SRCS 
    tasks/jt_getlastactivity.cpp
    tasks/jt_privatestorage.cpp
    tasks/jt_ahcommand.cpp
    tasks/jt_xsearch.cpp
    tasks/jt_xregister.cpp
    tasks/jt_pubsub.cpp
    tasks/mood.cpp
    tasks/privacylistitem.cpp
    tasks/privacylist.cpp
    tasks/privacymanager.cpp
    ${kopete_jabber_ui_SRCS}
    jabberprotocol.cpp
    jabberaccount.cpp
    jabberresource.cpp
    jabberresourcepool.cpp
    jabberbasecontact.cpp
    jabbercontact.cpp
    jabbergroupcontact.cpp
    jabbergroupmembercontact.cpp
    jabbercontactpool.cpp
    jabberformtranslator.cpp
    jabberxdatawidget.cpp
    jabberformlineedit.cpp
    jabberchatsession.cpp
    jabbergroupchatmanager.cpp
    jabberfiletransfer.cpp
    jabbercapabilitiesmanager.cpp
    jabbertransport.cpp
    jabberbookmarks.cpp
    jabberclient.cpp
    jabberbobcache.cpp
    xoauth2provider.cpp
    )
qt5_add_resources(kopete_jabber_PART_SRCS jabberprotocol.qrc)

add_library(kopete_jabber MODULE ${kopete_jabber_PART_SRCS})
generate_export_header(kopete_jabber BASE_NAME jabber)

target_link_libraries(kopete_jabber  KF5::KIOCore qca-qt5  kopete iris_kopete)

install(TARGETS kopete_jabber  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_jabber.desktop xmpp.protocol DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
