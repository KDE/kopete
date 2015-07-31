
#add_subdirectory( ui )

include_directories(
${KOPETE_INCLUDES}
${CMAKE_CURRENT_SOURCE_DIR}/ui/
${CMAKE_CURRENT_BINARY_DIR}/ui/
${CMAKE_CURRENT_SOURCE_DIR}/../
${CMAKE_CURRENT_SOURCE_DIR}/../liboscar/
${CMAKE_CURRENT_SOURCE_DIR}/../liboscar/utils
${CMAKE_CURRENT_SOURCE_DIR}/../liboscar/tasks
)


########### next target ###############

set(kopete_aim_ui_SRCS ui/aimaddcontactpage.cpp ui/aimeditaccountwidget.cpp)

ki18n_wrap_ui(kopete_aim_ui_SRCS
ui/aimaddcontactui.ui
ui/aiminfobase.ui
ui/aimjoinchatbase.ui
ui/aimeditaccountui.ui
)

set(kopete_aim_PART_SRCS
   ${kopete_aim_ui_SRCS}
   aimprotocol.cpp
   aimaccount.cpp
   aimcontact.cpp
   icqcontact.cpp
   aimstatusmanager.cpp
   aimuserinfo.cpp
   aimjoinchat.cpp
   aimchatsession.cpp )

add_library(kopete_aim MODULE ${kopete_aim_PART_SRCS})


target_link_libraries(kopete_aim
    kopete_oscar
    oscar
    kopete
    
    Qt5::Xml
    )

install(TARGETS kopete_aim  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_aim.desktop aim.protocol  DESTINATION ${SERVICES_INSTALL_DIR})




