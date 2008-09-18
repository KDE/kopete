INCLUDE(UsePkgConfig)

add_subdirectory( icons ) 

########### next target ###############

kde4_add_ui_files(kopete_meanwhile_ui_SRCS
   ui/meanwhileaddcontact.ui
   ui/meanwhileeditaccount.ui
)

set(kopete_meanwhile_PART_SRCS
   ${kopete_meanwhile_ui_SRCS}
   meanwhileaccount.cpp
   meanwhileaddcontactpage.cpp
   meanwhileplugin.cpp
   meanwhilecontact.cpp
   meanwhileprotocol.cpp
   meanwhileeditaccountwidget.cpp
   meanwhilesession.cpp )

### TODO - make these real configure checks
find_package(GLIB2)
PKGCONFIG(meanwhile _meanwhileIncDir _meanwhileLinkDir _meanwhileLinkFlags _meanwhileCflags)

FIND_LIBRARY(MEANWHILE_LIBRARY NAMES meanwhile
  PATHS
  ${_meanwhileLinkDir}
  NO_DEFAULT_PATH
)

### meanwhile.pc sets the wrong "includedir" variable - need to add meanwhile manually
#include_directories (${_meanwhileIncDir} ${GLIB2_INCLUDE_DIR})
include_directories (${_meanwhileIncDir}/meanwhile ${GLIB2_INCLUDE_DIR})

kde4_add_plugin(kopete_meanwhile ${kopete_meanwhile_PART_SRCS})

target_link_libraries(kopete_meanwhile
  ${KDE4_KDECORE_LIBS} ${GLIB2_LIBRARIES} ${MEANWHILE_LIBRARY}
  kopete)

install(TARGETS kopete_meanwhile  DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install( FILES kopete_meanwhile.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




