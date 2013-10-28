
include_directories( ${KOPETE_INCLUDES} )
if(XMMS_FOUND)
  include_directories(${XMMS_INCLUDE_DIRS} )
  include_directories( ${GLIB2_INCLUDE_DIR} )
  add_definitions(-DHAVE_XMMS)
endif(XMMS_FOUND)

########### next target ###############

set(kopete_nowlistening_PART_SRCS
   nowlisteningplugin.cpp
   nlkscd.cpp
   nlxmms.cpp
   nowlisteningguiclient.cpp
   nljuk.cpp
   nlamarok.cpp
   nlquodlibet.cpp
   nlkaffeine.cpp
   nlmpris.cpp
   nlmpris2.cpp
)

kde4_add_kcfg_files(kopete_nowlistening_PART_SRCS nowlisteningconfig.kcfgc )

kde4_add_plugin(kopete_nowlistening ${kopete_nowlistening_PART_SRCS})


target_link_libraries(kopete_nowlistening  ${KDE4_KIO_LIBS} kopete)

if(XMMS_FOUND)
   target_link_libraries( kopete_nowlistening ${XMMS_LDFLAGS} )
endif(XMMS_FOUND)

install(TARGETS kopete_nowlistening  DESTINATION ${PLUGIN_INSTALL_DIR})


########### next target ###############

set(kcm_kopete_nowlistening_PART_SRCS nowlisteningpreferences.cpp )

kde4_add_ui_files(kcm_kopete_nowlistening_PART_SRCS nowlisteningprefs.ui )

kde4_add_kcfg_files(kcm_kopete_nowlistening_PART_SRCS nowlisteningconfig.kcfgc )


kde4_add_plugin(kcm_kopete_nowlistening ${kcm_kopete_nowlistening_PART_SRCS})


target_link_libraries(kcm_kopete_nowlistening  ${KDE4_KUTILS_LIBS} ${KDE4_KDE3SUPPORT_LIBS})

install(TARGETS kcm_kopete_nowlistening  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES nowlisteningconfig.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kopete_nowlistening.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES nowlisteningui.rc nowlisteningchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete)
install( FILES kopete_nowlistening_config.desktop  DESTINATION ${SERVICES_INSTALL_DIR}/kconfiguredialog)




