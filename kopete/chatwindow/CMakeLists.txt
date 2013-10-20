
#add_subdirectory(tests)

add_subdirectory(a11y)

include_directories( 
${KOPETE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/../ 
)

link_directories( ${CMAKE_CURRENT_BINARY_DIR} )
########### next target ###############

set( kopetechatwindow_shared_SRCS 
chatmessagepart.cpp 
emoticonselector.cpp 
kopeteemoticonaction.cpp 
chattexteditpart.cpp
kopeterichtextwidget.cpp
kopetechatwindowstylemanager.cpp 
kopetechatwindowstyle.cpp )


kde4_add_kcfg_files(kopetechatwindow_shared_SRCS GENERATE_MOC kopetechatwindowsettings.kcfgc )
kde4_add_library(kopetechatwindow_shared SHARED ${kopetechatwindow_shared_SRCS})
target_link_libraries(kopetechatwindow_shared kopete ${KDE4_PHONON_LIBS} ${KDE4_KHTML_LIBS} ${KDE4_KUTILS_LIBS} )
set_target_properties(kopetechatwindow_shared PROPERTIES VERSION 1.0.0 SOVERSION 1 )
install(TARGETS kopetechatwindow_shared ${INSTALL_TARGETS_DEFAULT_ARGS})

########### next target ###############

set(chattexteditpart_PART_SRCS chattexteditpart.cpp )

kde4_add_plugin(chattexteditpart WITH_PREFIX ${chattexteditpart_PART_SRCS})


target_link_libraries(chattexteditpart kopete kopetechatwindow_shared ${KDE4_KPARTS_LIBS} )

install(TARGETS chattexteditpart  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kopete_chatwindow_PART_SRCS chatview.cpp kopetechatwindow.cpp chatmemberslistview.cpp)


kde4_add_plugin(kopete_chatwindow ${kopete_chatwindow_PART_SRCS})


target_link_libraries(kopete_chatwindow  ${KDE4_KDEUI_LIBS} kopete kopetechatwindow_shared ${KDE4_KHTML_LIBS} ${QT_QT3SUPPORT_LIBRARY})

install(TARGETS kopete_chatwindow  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kopete_emailwindow_PART_SRCS kopeteemailwindow.cpp )


kde4_add_plugin(kopete_emailwindow ${kopete_emailwindow_PART_SRCS})


target_link_libraries(kopete_emailwindow  ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_KHTML_LIBS} kopete kopetechatwindow_shared)

install(TARGETS kopete_emailwindow  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############


########### install files ###############

install( FILES kopeterichtexteditpartfull.rc  DESTINATION ${DATA_INSTALL_DIR}/kopeterichtexteditpart)
install( FILES chatwindow.desktop emailwindow.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES kopetechatwindow.rc kopeteemailwindow.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete)




