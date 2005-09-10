if include_smpppdcs
SMPPPDCS_SUBDIR=config
endif

METASOURCES = AUTO

SUBDIRS = icons $(SMPPPDCS_SUBDIR)

EXTRA_DIST = Changelog.smpppdcs

AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes)

kde_module_LTLIBRARIES = kopete_smpppdcs.la

kopete_smpppdcs_la_SOURCES = smpppdcsplugin.cpp detector.cpp onlineinquiry.cpp
kopete_smpppdcs_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kopete_smpppdcs_la_LIBADD  = ../../libkopete/libkopete.la -lcrypto

service_DATA = kopete_smpppdcs.desktop
servicedir = $(kde_servicesdir)

