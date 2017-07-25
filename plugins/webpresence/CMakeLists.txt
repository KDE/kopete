include_directories(${KOPETE_INCLUDES} ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR})

########### next target ###############

set(kopete_webpresence_PART_SRCS webpresenceplugin.cpp )

kconfig_add_kcfg_files(kopete_webpresence_PART_SRCS webpresenceconfig.kcfgc )

add_library(kopete_webpresence MODULE ${kopete_webpresence_PART_SRCS})

target_link_libraries(
	kopete_webpresence  
	KF5::KIOCore 
	kopete 
	${LIBXML2_LIBRARIES} 
	${LIBXSLT_LIBRARIES} 
	Qt5::Xml 
	KF5::I18n
)

install(TARGETS kopete_webpresence  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kcm_kopete_webpresence_PART_SRCS webpresencepreferences.cpp )

ki18n_wrap_ui(kcm_kopete_webpresence_PART_SRCS webpresenceprefs.ui )

kconfig_add_kcfg_files(kcm_kopete_webpresence_PART_SRCS webpresenceconfig.kcfgc )

add_library(kcm_kopete_webpresence MODULE ${kcm_kopete_webpresence_PART_SRCS})

target_link_libraries(
	kcm_kopete_webpresence  
	KF5::I18n 
	KF5::KIOCore 
	kopete 
)

install(TARGETS kcm_kopete_webpresence  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install(FILES webpresenceconfig.kcfg DESTINATION ${KDE_INSTALL_KCFGDIR})
install(FILES kopete_webpresence.desktop DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install(FILES webpresence_html.xsl webpresence_html_images.xsl webpresence_xhtml.xsl webpresence_xhtml_images.xsl DESTINATION ${KDE_INSTALL_DATADIR}/kopete/webpresence)
install(FILES kopete_webpresence_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)
