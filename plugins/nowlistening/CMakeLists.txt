
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

kconfig_add_kcfg_files(kopete_nowlistening_PART_SRCS nowlisteningconfig.kcfgc )

add_library(kopete_nowlistening MODULE ${kopete_nowlistening_PART_SRCS})


target_link_libraries(kopete_nowlistening
    KF5::KDELibs4Support
    KF5::KIOCore
    kopete
)

if(XMMS_FOUND)
   target_link_libraries( kopete_nowlistening ${XMMS_LDFLAGS} )
endif(XMMS_FOUND)

install(TARGETS kopete_nowlistening  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set(kcm_kopete_nowlistening_PART_SRCS nowlisteningpreferences.cpp )

ki18n_wrap_ui(kcm_kopete_nowlistening_PART_SRCS nowlisteningprefs.ui )

kconfig_add_kcfg_files(kcm_kopete_nowlistening_PART_SRCS nowlisteningconfig.kcfgc )


add_library(kcm_kopete_nowlistening MODULE ${kcm_kopete_nowlistening_PART_SRCS})


target_link_libraries(kcm_kopete_nowlistening
    KF5::KDELibs4Support
    KF5::KCMUtils
    Qt5::Widgets
    Qt5::Core
)

install(TARGETS kcm_kopete_nowlistening  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES nowlisteningconfig.kcfg  DESTINATION ${KDE_INSTALL_KCFGDIR})
install( FILES kopete_nowlistening.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
install( FILES nowlisteningui.rc nowlisteningchatui.rc  DESTINATION ${KDE_INSTALL_DATADIR}/kopete)
install( FILES kopete_nowlistening_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog)




