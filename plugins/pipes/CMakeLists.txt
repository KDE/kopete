
include_directories ( ${KOPETE_INCLUDES} )

#add_subdirectory ( tests )

########### next target ###############

find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS KCMUtils )

set (kopete_pipes_PART_SRCS 
 pipesplugin.cpp
 pipesconfig.cpp )

add_library(kopete_pipes MODULE ${kopete_pipes_PART_SRCS})

target_link_libraries( kopete_pipes KF5::KIOCore  kopete )

install( TARGETS kopete_pipes DESTINATION ${KDE_INSTALL_PLUGINDIR})


########### next target ###############

set ( kcm_kopete_pipes_PART_SRCS
  pipespreferences.cpp
  pipesdelegate.cpp
  pipesmodel.cpp
  pipesconfig.cpp )

ki18n_wrap_ui( kcm_kopete_pipes_PART_SRCS pipesprefsbase.ui )

add_library(kcm_kopete_pipes MODULE ${kcm_kopete_pipes_PART_SRCS} )

target_link_libraries( kcm_kopete_pipes
	KF5::KIOCore
	KF5::KCMUtils
	kopete
)

install( TARGETS kcm_kopete_pipes  DESTINATION ${KDE_INSTALL_PLUGINDIR} )

########### install files ###############
install( FILES kopete_pipes.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR} )
install( FILES kopete_pipes_config.desktop  DESTINATION ${KDE_INSTALL_KSERVICES5DIR}/kconfiguredialog )

