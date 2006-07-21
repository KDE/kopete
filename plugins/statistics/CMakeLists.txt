add_subdirectory( sqlite ) 

########### next target ###############

set(kopete_statistics_PART_SRCS 
   statisticsplugin.cpp 
   statisticsdb.cpp 
   statisticsdialog.cpp 
   statisticscontact.cpp )

kde4_automoc(${kopete_statistics_PART_SRCS})

kde4_add_ui_files(kopete_statistics_PART_SRCS statisticswidget.ui )

qt_add_dbus_interfaces(kopete_statistics_PART_SRCS org.kde.kopete.statisticsiface.xml)

kde4_add_plugin(kopete_statistics ${kopete_statistics_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_statistics )

target_link_libraries(kopete_statistics  ${KDE4_KDECORE_LIBS} kopete sqlite )

install(TARGETS kopete_statistics  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_statistics.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES statisticsui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_statistics)
install( FILES images/blue.png images/navy.png images/black.png images/gray.png  DESTINATION ${DATA_INSTALL_DIR}/kopete/pics/statistics)
