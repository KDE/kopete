AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT $(all_includes)

update_DATA = kopete-pluginloader.upd kopete-account-kconf_update.upd \
	kopete-pluginloader2.upd kopete-jabberproxytype-kconf_update.upd \
	kopete-jabberpriorityaddition-kconf_update.upd kopete-nameTracking.upd
update_SCRIPTS = kopete-pluginloader.pl kopete-account-kconf_update.sh \
	kopete-pluginloader2.sh kopete-jabberproxytype-kconf_update.sh \
	kopete-jabberpriorityaddition-kconf_update.sh kopete-account-0.10.pl
updatedir = $(kde_datadir)/kconf_update

# The Qt app cannot go into kde_datadir, that is not portable.
# install to kde_bindir/kconf_update_bin instead.
# KDE 3.2 will allow kconf_update scripts to run directly from there,
# but for us that's too late. Use the .sh script as a workaround.
kconf_PROGRAMS = kopete-account-kconf_update kopete-pluginloader2-kconf_update \
	kopete-nameTracking-kconf_update
kconfdir = $(libdir)/kconf_update_bin

kopete_account_kconf_update_SOURCES = kopete-account-kconf_update.cpp
kopete_account_kconf_update_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kopete_account_kconf_update_LDADD = $(LIB_QT)

kopete_pluginloader2_kconf_update_SOURCES = kopete-pluginloader2.cpp
kopete_pluginloader2_kconf_update_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kopete_pluginloader2_kconf_update_LDADD = $(LIB_QT)

kopete_nameTracking_kconf_update_SOURCES = kopete-nameTracking.cpp
kopete_nameTracking_kconf_update_LDFLAGS = $(all_libraries) $(KDE_RPATH)
kopete_nameTracking_kconf_update_LDADD = $(LIB_QT) $(LIB_KDECORE)

# vim: set noet:

