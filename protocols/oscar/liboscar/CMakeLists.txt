
add_subdirectory( tests ) 

include_directories(
${KOPETE_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR}/utils ${CMAKE_CURRENT_SOURCE_DIR}/tasks
)


########### next target ###############

set(liboscar_SRCS 
    oscarutils.cpp
    client.cpp 
    inputprotocolbase.cpp
    coreprotocol.cpp
    flapprotocol.cpp 
    snacprotocol.cpp
    oftprotocol.cpp
    ofttransfer.cpp
    transfer.cpp 
    oscarclientstream.cpp
    safedelete.cpp
    stream.cpp
    buffer.cpp
    rateclassmanager.cpp
    contactmanager.cpp
    contact.cpp
    rateclass.cpp
    rateclass.h
    connection.cpp
    userdetails.cpp
    oscartypeclasses.cpp
    oscarmessage.cpp
    icquserinfo.cpp
    oscarsettings.cpp
    connectionhandler.cpp
    oscarguid.cpp
    oscarmessageplugin.cpp
    oftmetatransfer.cpp
    xtrazxawayservice.cpp
    xtrazxservice.cpp
    xtrazxtraznotify.cpp
    xtrazxrandomizerservice.cpp
    filetransferhandler.cpp
    chatroomhandler.cpp
    tasks/task.cpp
    tasks/logintask.cpp
    tasks/oscarlogintask.cpp
    tasks/closeconnectiontask.cpp
    tasks/serverversionstask.cpp
    tasks/rateinfotask.cpp 
    tasks/errortask.cpp
    tasks/locationrightstask.cpp
    tasks/profiletask.cpp
    tasks/blmlimitstask.cpp
    tasks/servicesetuptask.cpp
    tasks/icbmparamstask.cpp
    tasks/prmparamstask.cpp
    tasks/ssiparamstask.cpp
    tasks/ssilisttask.cpp
    tasks/ssiactivatetask.cpp
    tasks/clientreadytask.cpp
    tasks/senddcinfotask.cpp
    tasks/sendidletimetask.cpp
    tasks/ownuserinfotask.cpp
    tasks/onlinenotifiertask.cpp
    tasks/ssimodifytask.cpp
    tasks/messagereceivertask.cpp
    tasks/messageacktask.cpp
    tasks/sendmessagetask.cpp
    tasks/icqtask.cpp
    tasks/offlinemessagestask.cpp
    tasks/ssiauthtask.cpp
    tasks/userinfotask.cpp
    tasks/icquserinfotask.cpp
    tasks/usersearchtask.cpp
    tasks/warningtask.cpp
    tasks/typingnotifytask.cpp
    tasks/buddyicontask.cpp
    tasks/serverredirecttask.cpp
    tasks/filetransfertask.cpp
    tasks/chatnavservicetask.cpp
    tasks/chatservicetask.cpp
    tasks/icquserinfoupdatetask.cpp
    tasks/icqchangepasswordtask.cpp
    tasks/icqtlvinforequesttask.cpp
    tasks/icqtlvinfoupdatetask.cpp
    tasks/chatroomtask.cpp
    utils/rtf.cc
)

kde4_add_library( oscar SHARED ${liboscar_SRCS} )

target_link_libraries( oscar ${KDE4_KDECORE_LIBS} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QT_QTXML_LIBRARY} )
set_target_properties( oscar PROPERTIES VERSION 1.0.0 SOVERSION 1 )


install(TARGETS oscar ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############





