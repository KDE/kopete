
METASOURCES = AUTO

AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_COMPAT $(all_includes)

update_DATA = kopete-account-kconf_update.upd
updatebin_PROGRAMS = kopete-account-kconf_update
updatedir = $(kde_datadir)/kconf_update

kopete_account_kconf_update_SOURCES = kopete-account-kconf_update.cpp
kopete_account_kconf_update_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
kopete_account_kconf_update_LDADD = $(LIB_QT)

# vim: set noet:

