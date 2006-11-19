
add_subdirectory( icons ) 
add_subdirectory( libgroupwise ) 
add_subdirectory( ui ) 

include_directories( ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise/tasks ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise/qca/src ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/ui  )


########### next target ###############

set(kopete_groupwise_PART_SRCS 
   gwprotocol.cpp 
   gwcontact.cpp 
   gwaccount.cpp 
   gwbytestream.cpp 
   gwconnector.cpp 
   gwmessagemanager.cpp 
   gwcontactlist.cpp )

kde4_automoc(${kopete_groupwise_PART_SRCS})

kde4_add_plugin(kopete_groupwise ${kopete_groupwise_PART_SRCS})


target_link_libraries(kopete_groupwise  ${KDE4_KDECORE_LIBS} kopetegroupwiseui groupwise kopete )

install(TARGETS kopete_groupwise  DESTINATION ${PLUGIN_INSTALL_DIR})


########### install files ###############

install( FILES kopete_groupwise.desktop  DESTINATION ${SERVICES_INSTALL_DIR})
install( FILES gwchatui.rc  DESTINATION ${DATA_INSTALL_DIR}/kopete_groupwise)




