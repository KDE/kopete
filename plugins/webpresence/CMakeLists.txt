include_directories(${KOPETE_INCLUDES} ${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR})

########### next target ###############

set(kopete_webpresence_PART_SRCS webpresenceplugin.cpp )

kde4_add_kcfg_files(kopete_webpresence_PART_SRCS webpresenceconfig.kcfgc )

kde4_add_plugin(kopete_webpresence ${kopete_webpresence_PART_SRCS})

target_link_libraries(kopete_webpresence  ${KDE4_KIO_LIBS} kopete ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${QT_QTXML_LIBRARY})

install(TARGETS kopete_webpresence  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_webpresence_PART_SRCS webpresencepreferences.cpp )

kde4_add_ui_files(kcm_kopete_webpresence_PART_SRCS webpresenceprefs.ui )

kde4_add_kcfg_files(kcm_kopete_webpresence_PART_SRCS webpresenceconfig.kcfgc )

kde4_add_plugin(kcm_kopete_webpresence ${kcm_kopete_webpresence_PART_SRCS})

target_link_libraries(kcm_kopete_webpresence  ${KDE4_KUTILS_LIBS} ${KDE4_KIO_LIBS} kopete )

install(TARGETS kcm_kopete_webpresence  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install(FILES webpresenceconfig.kcfg DESTINATION ${KCFG_INSTALL_DIR})
install(FILES kopete_webpresence.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(FILES webpresence_html.xsl webpresence_html_images.xsl webpresence_xhtml.xsl webpresence_xhtml_images.xsl DESTINATION ${DATA_INSTALL_DIR}/kopete/webpresence)
install(FILES kopete_webpresence_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)
