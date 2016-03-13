
add_subdirectory( icons )

add_definitions(${LIBMEANWHILE_DEFINITIONS})
include_directories(${LIBMEANWHILE_INCLUDE_DIR} ${GLIB2_INCLUDE_DIR})

########### next target ###############

set(kopete_meanwhile_PART_SRCS
   meanwhileaccount.cpp
   meanwhileaddcontactpage.cpp
   meanwhileplugin.cpp
   meanwhilecontact.cpp
   meanwhileprotocol.cpp
   meanwhileeditaccountwidget.cpp
   meanwhilesession.cpp )

kde4_add_ui_files(kopete_meanwhile_PART_SRCS
   ui/meanwhileaddcontact.ui
   ui/meanwhileeditaccount.ui
)

kde4_add_plugin(kopete_meanwhile ${kopete_meanwhile_PART_SRCS})

target_link_libraries(kopete_meanwhile
  ${KDE4_KDECORE_LIBS} ${GLIB2_LIBRARIES} ${LIBMEANWHILE_LIBRARY}
  ${KDE4_KDEUI_LIBS} ${QT_QTNETWORK_LIBRARY}
  kopete)

install(TARGETS kopete_meanwhile DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kopete_meanwhile.desktop DESTINATION ${SERVICES_INSTALL_DIR})




