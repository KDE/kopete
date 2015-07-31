add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)


add_subdirectory( icons ) 

include_directories( ${KOPETE_INCLUDES} ${LIBGADU_INCLUDE_DIRS} }
)


########### next target ###############

find_package(KF5 ${KF5_VERSION} REQUIRED COMPONENTS KIO)
find_package(Qt5 ${REQUIRED_QT_VERSION} REQUIRED COMPONENTS Network)

set(kopete_gaduui_SRCS)

ki18n_wrap_ui(kopete_gaduui_SRCS
   ui/gadueditaccountui.ui
   ui/gaduadd.ui 
   ui/gaduawayui.ui 
   ui/gaduregisteraccountui.ui 
   ui/gadusearch.ui 
)

set(kopete_gadu_PART_SRCS 
   ${kopete_gaduui_SRCS} 
   gaduaway.cpp 
   gadueditcontact.cpp 
   gaducommands.cpp 
   gadueditaccount.cpp 
   gadusession.cpp 
   gaducontact.cpp 
   gaduaddcontactpage.cpp 
   gaduprotocol.cpp 
   gaduaccount.cpp 
   gadupubdir.cpp 
   gaduregisteraccount.cpp 
   gaducontactlist.cpp 
   gadurichtextformat.cpp 
   gadudccserver.cpp 
   gadudcctransaction.cpp 
   gadudcc.cpp )


add_library(kopete_gadu MODULE ${kopete_gadu_PART_SRCS})


target_link_libraries(kopete_gadu KF5::KIOCore Qt5::Network ${KDE4_KDE3SUPPORT_LIBS} ${LIBGADU_LIBRARIES} kopete )

install(TARGETS kopete_gadu  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_gadu.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




