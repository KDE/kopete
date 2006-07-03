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

install_targets(${PLUGIN_INSTALL_DIR} kopete_statistics )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_statistics.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete_statistics FILES statisticsui.rc )
install_files( ${DATA_INSTALL_DIR}/kopete/pics/statistics FILES images/blue.png images/navy.png images/black.png images/gray.png )
