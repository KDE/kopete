if(NOT WIN32)
	install( PROGRAMS kopete-account-kconf_update.sh kopete-jabberpriorityaddition-kconf_update.sh kopete-pluginloader2.sh kopete-jabberproxytype-kconf_update.sh DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
	install(FILES kopete-account-kconf_update.upd kopete-jabberproxytype-kconf_update.upd kopete-jabberpriorityaddition-kconf_update.upd kopete-pluginloader2.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})
endif(NOT WIN32)


install( FILES kopete-pluginloader.upd  kopete-nameTracking.upd  kopete-initialstatus.upd  DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

install( PROGRAMS kopete-pluginloader.pl
	kopete-account-0.10.pl kopete-initialstatus.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR})


set( kopeteclupdate_SRCS kopete-contactlist-v2-update.cpp )

kde4_add_executable(kopete-contactlist-v2-updater NOGUI ${kopeteclupdate_SRCS})

target_link_libraries(kopete-contactlist-v2-updater
                      ${KDE4_KDEUI_LIBS} ${QT_QTXML_LIBRARY})
install( TARGETS kopete-contactlist-v2-updater DESTINATION
         ${BIN_INSTALL_DIR}/kconf_update_bin)
install( FILES kopete-contactlist-v2-update.upd DESTINATION ${KCONF_UPDATE_INSTALL_DIR})


