ADD_SUBDIRECTORY( wrappers ) 

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${KDE3_INCLUDE_DIR} ${QT_INCLUDE_DIR}  )


########### next target ###############

SET(kopete_javascript_PART_SRCS
javascriptplugin.cpp
javascriptconfig.cpp
)

KDE3_AUTOMOC(${kopete_javascript_PART_SRCS})

KDE3_ADD_KPART(kopete_javascript ${kopete_javascript_PART_SRCS})

TARGET_LINK_LIBRARIES(kopete_javascript  ${QT_AND_KDECORE_LIBS} kopete kjsembed wrappers )

INSTALL_TARGETS(/lib/kde3 kopete_javascript )


########### next target ###############

SET(kcm_kopete_javascript_PART_SRCS
javascriptpreferences.cpp
javascriptconfig.cpp
)

KDE3_AUTOMOC(${kcm_kopete_javascript_PART_SRCS})

SET( kcm_kopete_javascript_UI
javascriptprefsbase.ui
)

KDE3_ADD_UI_FILES(kcm_kopete_javascript_PART_SRCS ${kcm_kopete_javascript_UI} )

KDE3_ADD_KPART(kcm_kopete_javascript ${kcm_kopete_javascript_PART_SRCS})

TARGET_LINK_LIBRARIES(kcm_kopete_javascript  ${QT_AND_KDECORE_LIBS} knewstuff kopete )

INSTALL_TARGETS(/lib/kde3 kcm_kopete_javascript )


########### install files ###############

INSTALL_FILES( /share/services FILES  kopete_javascript.desktop )
INSTALL_FILES( /share/services/kconfiguredialog FILES  kopete_javascript_config.desktop )



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#SUBDIRS = wrappers
#INCLUDES = $(KOPETE_INCLUDES) $(all_includes)
#
#kde_module_LTLIBRARIES = kopete_javascript.la kcm_kopete_javascript.la
#
#kopete_javascript_la_SOURCES     = javascriptplugin.cpp javascriptconfig.cpp
#kopete_javascript_la_LDFLAGS     = -module -no-undefined $(KDE_PLUGIN )
#kopete_javascript_la_LIBADD      = ../../libkopete/libkopete.la -lkjsembed \
#					wrappers/libwrappers.la
#
#kcm_kopete_javascript_la_SOURCES = javascriptprefsbase.ui   \
#				javascriptpreferences.cpp javascriptconfig.cpp
#
#kcm_kopete_javascript_la_LDFLAGS = -module -no-undefined $(all_libraries) -lknewstuff
#kcm_kopete_javascript_la_LIBADD  = ../../libkopete/libkopete.la
#
#service_DATA = kopete_javascript.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_javascript_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
