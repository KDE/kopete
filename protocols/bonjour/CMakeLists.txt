project ( kopete-bonjour )

add_definitions(-DKDE_DEFAULT_DEBUG_AREA=14220)

include_directories(${KDEPIMLIBS_INCLUDE_DIRS})

add_subdirectory( icons ) 

########### next target ###############

find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS KIO DNSSD)

set(kopete_bonjour_PART_SRCS 
    bonjourprotocol.cpp
    bonjourcontact.cpp
    bonjouraccount.cpp
    bonjouraddcontactpage.cpp
    bonjoureditaccountwidget.cpp
    bonjourcontactconnection.cpp
    )

ki18n_wrap_ui(kopete_bonjour_PART_SRCS
    bonjouraddui.ui
    bonjouraccountpreferences.ui
    )

add_library(kopete_bonjour MODULE ${kopete_bonjour_PART_SRCS})

target_link_libraries(kopete_bonjour KF5::KIOCore KF5::IdentityManagement KF5::DNSSD kopete)
if(NOT DISABLE_VIDEOSUPPORT)
    target_link_libraries(kopete_bonjour kopete_videodevice)
endif(NOT DISABLE_VIDEOSUPPORT)

install(TARGETS kopete_bonjour  DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### install files ###############

install( FILES kopete_bonjour.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR})
