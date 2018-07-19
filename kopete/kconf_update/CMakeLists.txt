if(NOT WIN32)
    install( PROGRAMS kopete-account-kconf_update.sh kopete-jabberpriorityaddition-kconf_update.sh kopete-pluginloader2.sh kopete-jabberproxytype-kconf_update.sh DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
    install(FILES kopete-account-kconf_update.upd kopete-jabberproxytype-kconf_update.upd kopete-jabberpriorityaddition-kconf_update.upd kopete-pluginloader2.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})
endif(NOT WIN32)


install( FILES kopete-pluginloader.upd  kopete-nameTracking.upd  kopete-initialstatus.upd  kopete-gaim_to_pidgin_style.upd kopete-update_icq_server.upd kopete-update_icq_ssl.upd DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})

install( PROGRAMS kopete-pluginloader.pl kopete-update_icq_server.pl kopete-update_icq_ssl.pl
    kopete-account-0.10.pl kopete-initialstatus.pl kopete-gaim_to_pidgin_style.pl
    DESTINATION ${KDE_INSTALL_KCONFUPDATEDIR})


