add_definitions(-DQT3_SUPPORT -DQT3_SUPPORT_WARNINGS)

KDE4_NO_ENABLE_FINAL(kopete)

add_subdirectory( icons ) 

include_directories( ${KOPETE_INCLUDES} ${LIBGADU_INCLUDE_DIRS} }
)


########### next target ###############

set(kopete_gaduui_SRCS)

kde4_add_ui_files(kopete_gaduui_SRCS
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


kde4_add_plugin(kopete_gadu ${kopete_gadu_PART_SRCS})


target_link_libraries(kopete_gadu ${KDE4_KIO_LIBS} ${QT_QTNETWORK_LIBRARY} ${KDE4_KDE3SUPPORT_LIBS} ${LIBGADU_LIBRARIES} kopete )

install(TARGETS kopete_gadu  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_gadu.desktop  DESTINATION ${SERVICES_INSTALL_DIR})




