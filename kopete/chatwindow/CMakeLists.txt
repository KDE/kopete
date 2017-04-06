
#add_subdirectory(tests)

add_subdirectory(a11y)

include_directories( 
    ${KOPETE_INCLUDES}
    ${CMAKE_CURRENT_SOURCE_DIR}/../
    )

link_directories( ${CMAKE_CURRENT_BINARY_DIR} )
########### next target ###############
set(kopetechatwindow_shared_common_SRCS)
ecm_qt_declare_logging_category(kopetechatwindow_shared_common_SRCS HEADER kopetechatwindow_debug.h IDENTIFIER KOPETE_CHATEWINDOW_LOG CATEGORY_NAME org.kde.kopete.chatwindow)

set( kopetechatwindow_shared_SRCS 
    chatmessagepart.cpp
    emoticonselector.cpp
    kopeteemoticonaction.cpp
    chattexteditpart.cpp
    kopeterichtextwidget.cpp
    kopetechatwindowstylemanager.cpp
    kopetechatwindowstyle.cpp
    ${kopetechatwindow_shared_common_SRCS})


kconfig_add_kcfg_files(kopetechatwindow_shared_SRCS GENERATE_MOC kopetechatwindowsettings.kcfgc )
add_library(kopetechatwindow_shared SHARED ${kopetechatwindow_shared_SRCS})
generate_export_header(kopetechatwindow_shared BASE_NAME kopetechatwindow_shared)
target_link_libraries(kopetechatwindow_shared kopete Phonon::phonon4qt5 KF5::KHtml  )
set_target_properties(kopetechatwindow_shared PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopetechatwindow_shared ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(chattexteditpart_PART_SRCS chattexteditpart.cpp )

add_library(chattexteditpart MODULE ${chattexteditpart_PART_SRCS})

generate_export_header(chattexteditpart BASE_NAME chattexteditpart)
target_link_libraries(chattexteditpart kopete kopetechatwindow_shared KF5::Parts )

install(TARGETS chattexteditpart  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kopete_chatwindow_PART_SRCS chatview.cpp kopetechatwindow.cpp chatmemberslistview.cpp ${kopetechatwindow_shared_common_SRCS})


add_library(kopete_chatwindow MODULE ${kopete_chatwindow_PART_SRCS})

target_link_libraries(kopete_chatwindow   kopete kopetechatwindow_shared KF5::KHtml)

install(TARGETS kopete_chatwindow  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kopete_emailwindow_PART_SRCS kopeteemailwindow.cpp )


add_library(kopete_emailwindow MODULE ${kopete_emailwindow_PART_SRCS})


target_link_libraries(kopete_emailwindow   KF5::Parts KF5::KHtml kopete kopetechatwindow_shared)

install(TARGETS kopete_emailwindow  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############


########### install files ###############

install( FILES kopeterichtexteditpartfull.rc  DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kopeterichtexteditpart)
install( FILES chatwindow.desktop emailwindow.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES kopetechatwindow.rc kopeteemailwindow.rc  DESTINATION ${KDE_INSTALL_KXMLGUI5DIR}/kopete)




