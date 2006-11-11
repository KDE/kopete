
install( FILES kopete-pluginloader.upd kopete-account-kconf_update.upd  	kopete-pluginloader2.upd kopete-jabberproxytype-kconf_update.upd  	kopete-jabberpriorityaddition-kconf_update.upd kopete-nameTracking.upd  DESTINATION ${KCONF_UPDATE_INSTALL_DIR})

install( PROGRAMS kopete-pluginloader.pl kopete-account-kconf_update.sh kopete-pluginloader2.sh
	kopete-jabberproxytype-kconf_update.sh kopete-jabberpriorityaddition-kconf_update.sh
	kopete-account-0.10.pl DESTINATION ${KCONF_UPDATE_INSTALL_DIR})


#TODO kconf_program ?

