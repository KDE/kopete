
add_subdirectory( tests ) 

include_directories(
${KOPETE_INCLUDES} 
)


########### next target ###############

set(liboscar_SRCS 
oscarutils.cpp client.cpp task.cpp connector.cpp 
inputprotocolbase.cpp coreprotocol.cpp flapprotocol.cpp snacprotocol.cpp oftprotocol.cpp ofttransfer.cpp transfer.cpp rtf.cc 
bytestream.cpp oscarclientstream.cpp safedelete.cpp stream.cpp oscarconnector.cpp 
oscarbytestream.cpp buffer.cpp md5.c logintask.cpp aimlogintask.cpp icqlogintask.cpp 
closeconnectiontask.cpp rateclassmanager.cpp serverversionstask.cpp rateinfotask.cpp 
errortask.cpp locationrightstask.cpp profiletask.cpp blmlimitstask.cpp 
servicesetuptask.cpp icbmparamstask.cpp contactmanager.cpp contact.cpp rateclass.cpp rateclass.h 
prmparamstask.cpp ssiparamstask.cpp ssilisttask.cpp ssiactivatetask.cpp 
clientreadytask.cpp senddcinfotask.cpp sendidletimetask.cpp ownuserinfotask.cpp 
connection.cpp onlinenotifiertask.cpp userdetails.cpp ssimodifytask.cpp 
oscartypeclasses.cpp oscarmessage.cpp messagereceivertask.cpp sendmessagetask.cpp icqtask.cpp 
offlinemessagestask.cpp ssiauthtask.cpp userinfotask.cpp icquserinfo.cpp icquserinfotask.cpp 
usersearchtask.cpp warningtask.cpp changevisibilitytask.cpp typingnotifytask.cpp 
buddyicontask.cpp serverredirecttask.cpp oscarsettings.cpp filetransfertask.cpp
localcontactlisttask.cpp chatnavservicetask.cpp connectionhandler.cpp chatservicetask.cpp oscarguid.cpp 
icquserinfoupdatetask.cpp icqchangepasswordtask.cpp
)

kde4_automoc(${liboscar_SRCS} )

kde4_add_library( oscar SHARED ${liboscar_SRCS} )

target_link_libraries( oscar ${KDE4_KDECORE_LIBS} ${QT_QTNETWORK_LIBRARY} )

install(TARGETS oscar  DESTINATION ${LIB_INSTALL_DIR})

########### install files ###############





