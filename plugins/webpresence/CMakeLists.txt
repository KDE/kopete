
include_directories( 
${KOPETE_INCLUDES} ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR}
)


########### next target ###############

set(kopete_webpresence_PART_SRCS webpresenceplugin.cpp )

kde4_automoc(${kopete_webpresence_PART_SRCS})

kde4_add_plugin(kopete_webpresence ${kopete_webpresence_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kopete_webpresence )

target_link_libraries(kopete_webpresence  ${KDE4_KDECORE_LIBS} kopete ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} )

install_targets(${PLUGIN_INSTALL_DIR} kopete_webpresence )


########### next target ###############

set(kcm_kopete_webpresence_PART_SRCS webpresencepreferences.cpp )

kde4_add_ui_files(kcm_kopete_webpresence_PART_SRCS webpresenceprefs.ui )

kde4_automoc(${kcm_kopete_webpresence_PART_SRCS})

kde4_add_plugin(kcm_kopete_webpresence ${kcm_kopete_webpresence_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kcm_kopete_webpresence )

target_link_libraries(kcm_kopete_webpresence  ${KDE4_KUTILS_LIBS} kopete )

install_targets(${PLUGIN_INSTALL_DIR} kcm_kopete_webpresence )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES kopete_webpresence.desktop )
install_files( ${DATA_INSTALL_DIR}/kopete/webpresence FILES webpresence_html.xsl 	      webpresence_html_images.xsl 	      webpresence_xhtml.xsl 	      webpresence_xhtml_images.xsl )
install_files( ${SERVICES_INSTALL_DIR}/kconfiguredialog FILES kopete_webpresence_config.desktop )




#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#AM_CPPFLAGS = $(KOPETE_INCLUDES) $(all_includes) $(LIBXML_CFLAGS) $(LIBXSLT_CFLAGS)
#
#kde_module_LTLIBRARIES = kopete_webpresence.la kcm_kopete_webpresence.la
#
#kopete_webpresence_la_SOURCES     = webpresenceplugin.cpp
#
#kopete_webpresence_la_LDFLAGS     = -module  -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kopete_webpresence_la_LIBADD      = ../../libkopete/libkopete.la $(LIBXML_LIBS) $(LIBXSLT_LIBS)
#
#kcm_kopete_webpresence_la_SOURCES = webpresencepreferences.cpp webpresenceprefs.ui
#kcm_kopete_webpresence_la_LDFLAGS = -module -no-undefined $(KDE_PLUGIN) $(all_libraries)
#kcm_kopete_webpresence_la_LIBADD  = ../../libkopete/libkopete.la  $(LIB_KUTILS)
#
#service_DATA = kopete_webpresence.desktop
#servicedir = $(kde_servicesdir)
#
#kcm_DATA = kopete_webpresence_config.desktop
#kcmdir = $(kde_servicesdir)/kconfiguredialog
#
#mydata_DATA = webpresence_html.xsl\
#	      webpresence_html_images.xsl\
#	      webpresence_xhtml.xsl\
#	      webpresence_xhtml_images.xsl
#mydatadir = $(kde_datadir)/kopete/webpresence
#EXTRA_DIST = $(mydata_DATA)
