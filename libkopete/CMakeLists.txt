
include(CheckIncludeFile)
check_include_file(valgrind/valgrind.h HAVE_VALGRIND_H)
if(HAVE_VALGRIND_H)
    add_definitions(-DHAVE_VALGRIND_H)
endif(HAVE_VALGRIND_H)

add_subdirectory( private )
if(NOT DISABLE_VIDEOSUPPORT)
    add_subdirectory( avdevice )
endif(NOT DISABLE_VIDEOSUPPORT)

include_directories( ${KOPETE_INCLUDES} )

########### next target ###############

set(kopete_private_SRCS
    private/kopeteemoticons.cpp
    private/kopeteutils_private.cpp
    private/kopeteviewmanager.cpp
    private/kopeteactivenotification.cpp
    )

set(kopete_private_SRCS ${kopete_private_SRCS} private/kopeteidleplatform_dummy.cpp )
if (Q_WS_X11)
    if (X11_Xss_FOUND)
        set(kopete_private_SRCS ${kopete_private_SRCS} private/kopeteidleplatform_x11.cpp )
    else (X11_Xss_FOUND)
        set(kopete_private_SRCS ${kopete_private_SRCS} private/kopeteidleplatform_dummy.cpp )
    endif (X11_Xss_FOUND)
endif (Q_WS_X11)
if (Q_WS_MAC)
    set(kopete_private_SRCS ${kopete_private_SRCS} private/kopeteidleplatform_mac.cpp )
endif (Q_WS_MAC)
if (Q_WS_WIN)
    set(kopete_private_SRCS ${kopete_private_SRCS} private/kopeteidleplatform_win.cpp )
endif (Q_WS_WIN)

set(kopete_ui_SRCS
    ui/accountselector.cpp
    ui/addcontactpage.cpp
    ui/addressbooklinkwidget.cpp
    ui/addressbookselectordialog.cpp
    ui/addressbookselectorwidget.cpp
    ui/addresseeitem.cpp
    ui/avatardialog.cpp
    ui/avatarwebcamdialog.cpp
    ui/avatarselectorwidget.cpp
    ui/contactaddednotifydialog.cpp
    ui/collapsiblewidget.cpp
    ui/editaccountwidget.cpp
    ui/kopetecontactaction.cpp
    ui/kopeteinfodialog.cpp
    ui/kopetelistview.cpp
    ui/kopetelistviewitem.cpp
    ui/kopetelistviewsearchline.cpp
    ui/kopetepasswordwidget.cpp
    ui/kopetestdaction.cpp
    ui/kopeteview.cpp
    ui/kopeteviewplugin.cpp
    ui/metacontactselectorwidget.cpp
    ui/webcamwidget.cpp
    )

set(kopete_contactlist_SRCS
    contactlist/kopetecontactliststorage.cpp
    contactlist/xmlcontactstorage.cpp
    )

set(kopete_tasks_SRCS
    tasks/kopetetask.cpp
    tasks/kopetecontacttaskbase.cpp
    tasks/kopetedeletecontacttask.cpp
    )

set(kopete_LIB_SRCS
    ${kopete_private_SRCS}
    ${kopete_ui_SRCS}
    ${kopete_contactlist_SRCS}
    ${kopete_tasks_SRCS}
    kabcpersistence.cpp
    kopeteaccount.cpp
    kopeteaccountmanager.cpp
    kopeteavatarmanager.cpp
    kopeteblacklister.cpp
    kopetechatsession.cpp
    kopetechatsessionmanager.cpp
    kopetecommand.cpp
    kopetecommandhandler.cpp
    kopetecontact.cpp
    kopetecontactlist.cpp
    kopetecontactlistelement.cpp
    kopeteproperty.cpp
    kopeteglobal.cpp
    kopetegroup.cpp
    kopeteidentity.cpp
    kopeteidentitymanager.cpp
    kopetemessage.cpp
    kopetemessageevent.cpp
    kopetemessagehandlerchain.cpp
    kopetemessagehandler.cpp
    kopetemetacontact.cpp
    #kopetefileengine.cpp
    kopetemimetypehandler.cpp
    kopeteonlinestatus.cpp
    kopeteonlinestatusmanager.cpp
    kopetepassword.cpp
    kopetepasswordedaccount.cpp
    kopetepicture.cpp
    kopeteplugin.cpp
    kopetepluginmanager.cpp
    kopeteproperties.cpp
    kopetepropertycontainer.cpp
    kopeteprotocol.cpp
    kopetesimplemessagehandler.cpp
    kopetestatusmessage.cpp
    kopetetransfermanager.cpp
    kopeteuiglobal.cpp
    kopeteutils.cpp
    kopetewalletmanager.cpp
    networkstatuscommon.h
    chatsessionmemberslistmodel.cpp
    kopetestatusmanager.cpp
    kopetestatusitems.cpp
    kopeteidletimer.cpp
    kopeteinfoeventmanager.cpp
    kopeteinfoevent.cpp
    kopeteaddedinfoevent.cpp
    kopetesockettimeoutwatcher.cpp
    # REMOVED FOR NOW
    #   managedconnectionaccount.cpp

    )

ki18n_wrap_ui(kopete_LIB_SRCS
    ui/addressbooklinkwidget_base.ui
    ui/addressbookselectorwidget_base.ui
    ui/avatarselectorwidget.ui
    ui/contactaddednotifywidget.ui
    ui/kopeteawaydialogbase.ui
    ui/kopetepasswordwidgetbase.ui
    ui/metacontactselectorwidget_base.ui
    )

kconfig_add_kcfg_files(kopete_LIB_SRCS GENERATE_MOC
    kopeteappearancesettings.kcfgc
    kopetebehaviorsettings.kcfgc
    kopetestatussettings.kcfgc
    )
qt5_add_resources(kopete_LIB_SRCS libkopete.qrc)

ecm_qt_declare_logging_category(kopete_LIB_SRCS HEADER libkopete_debug.h IDENTIFIER LIBKOPETE_LOG CATEGORY_NAME org.kde.kopete.libkopete)

find_package(KF5Contacts ${KCONTACTS_LIB_VERSION} CONFIG REQUIRED)
add_library(kopete SHARED ${kopete_LIB_SRCS})
generate_export_header(kopete BASE_NAME libkopete)

target_link_libraries(kopete
    KF5::Contacts
    KF5::Emoticons
    KF5::KCMUtils
    KF5::I18n
    KF5::KIOCore
    KF5::KIOWidgets
    KF5::Archive
    KF5::ConfigCore
    KF5::ConfigGui
    KF5::Parts
    KF5::Wallet
    KF5::KDELibs4Support
    )

if(NOT DISABLE_VIDEOSUPPORT)
    target_link_libraries(kopete kopete_videodevice)
endif(NOT DISABLE_VIDEOSUPPORT)

if(Q_WS_X11 AND X11_Xss_FOUND)
    target_link_libraries(kopete ${X11_X11_LIB} ${X11_Xss_LIB})
endif(Q_WS_X11 AND X11_Xss_FOUND)

set_target_properties(kopete PROPERTIES
    VERSION 1.0.0
    SOVERSION 1
    )
if(MSVC)
    set_target_properties(kopete PROPERTIES
        OUTPUT_NAME libkopete
        )
endif(MSVC)
install(TARGETS kopete ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})


########### install files ###############

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/ui_kopeteawaydialogbase.h
    ${CMAKE_CURRENT_BINARY_DIR}/ui_kopetepasswordwidgetbase.h
    ui/accountselector.h
    ui/addcontactpage.h
    ui/addressbookselectordialog.h
    ui/addressbookselectorwidget.h
    ui/avatardialog.h
    ui/avatarselectorwidget.h
    ui/contactaddednotifydialog.h
    ui/editaccountwidget.h
    ui/kopetecontactaction.h
    ui/kopeteinfodialog.h
    ui/kopetelistview.h
    ui/kopetelistviewitem.h
    ui/kopetelistviewsearchline.h
    ui/kopetepasswordwidget.h
    ui/kopetestdaction.h
    ui/kopeteview.h
    ui/kopeteviewplugin.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kopete/ui COMPONENT Devel )


install( FILES
    kopeteappearancesettings.kcfg
    kopetebehaviorsettings.kcfg
    kopetestatussettings.kcfg
    DESTINATION ${KDE_INSTALL_KCFGDIR})

install( FILES kopeteplugin.desktop kopeteprotocol.desktop kopeteui.desktop  DESTINATION ${KDE_INSTALL_KSERVICETYPES5DIR})

install( FILES
    ${CMAKE_CURRENT_BINARY_DIR}/kopeteappearancesettings.h
    ${CMAKE_CURRENT_BINARY_DIR}/kopetebehaviorsettings.h
    ${CMAKE_CURRENT_BINARY_DIR}/kopetestatussettings.h
    kabcpersistence.h
    kopeteaccount.h
    kopeteaccountmanager.h
    kopeteaddedinfoevent.h
    kopeteavatarmanager.h
    kopeteblacklister.h
    kopetechatsession.h
    kopetechatsessionmanager.h
    kopetecommand.h
    kopetecommandhandler.h
    kopetecontact.h
    kopetecontactlistelement.h
    kopetecontactlist.h
    kopeteproperty.h
    ${CMAKE_CURRENT_BINARY_DIR}/libkopete_export.h
    kopeteglobal.h
    kopetegroup.h
    kopeteidentity.h
    kopeteidentitymanager.h
    kopeteinfoevent.h
    kopetemessageevent.h
    kopetemessage.h
    kopetemessagehandlerchain.h
    kopetemessagehandler.h
    kopetemetacontact.h
    kopetemimetypehandler.h
    kopeteonlinestatus.h
    kopeteonlinestatusmanager.h
    kopetepasswordedaccount.h
    kopetepassword.h
    kopetepicture.h
    kopeteplugin.h
    kopetepluginmanager.h
    kopeteprotocol.h
    kopetepropertycontainer.h
    kopetesimplemessagehandler.h
    kopetestatusmessage.h
    kopetetransfermanager.h
    kopeteuiglobal.h
    kopeteutils.h
    kopeteversion.h
    kopetestatusmanager.h
    tasks/kopetetask.h
    tasks/kopetecontacttaskbase.h
    tasks/kopetedeletecontacttask.h
    private/kopeteemoticons.h
    DESTINATION ${KDE_INSTALL_INCLUDEDIR}/kopete COMPONENT Devel)

install( FILES org.kde.kopete.Client.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} )
