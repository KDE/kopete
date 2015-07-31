
add_subdirectory( icons )

add_definitions(${LIBMEANWHILE_DEFINITIONS})
include_directories(${LIBMEANWHILE_INCLUDE_DIR})

########### next target ###############

set(kopete_meanwhile_PART_SRCS
   meanwhileaccount.cpp
   meanwhileaddcontactpage.cpp
   meanwhileplugin.cpp
   meanwhilecontact.cpp
   meanwhileprotocol.cpp
   meanwhileeditaccountwidget.cpp
   meanwhilesession.cpp )

ki18n_wrap_ui(kopete_meanwhile_PART_SRCS
   ui/meanwhileaddcontact.ui
   ui/meanwhileeditaccount.ui
)

add_library(kopete_meanwhile MODULE ${kopete_meanwhile_PART_SRCS})

target_link_libraries(kopete_meanwhile
  KF5::KDELibs4Support ${GLIB2_LIBRARIES} ${LIBMEANWHILE_LIBRARY}
   Qt5::Network
  kopete)

install(TARGETS kopete_meanwhile DESTINATION ${PLUGIN_INSTALL_DIR})

########### install files ###############

install(FILES kopete_meanwhile.desktop DESTINATION ${SERVICES_INSTALL_DIR})




