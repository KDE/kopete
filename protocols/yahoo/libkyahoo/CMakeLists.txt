include_directories( ${JASPER_INCLUDE_DIR} )

add_subdirectory( tests ) 



########### next target ###############
set(libkyahoo_SRCS 
client.cpp 
task.cpp 
connector.cpp 
inputprotocolbase.cpp 
ymsgprotocol.cpp 
ymsgtransfer.cpp 
transfer.cpp 
yahoobytestream.cpp 
bytestream.cpp 
yahooclientstream.cpp 
yahooconnector.cpp 
safedelete.cpp 
stream.cpp 
sha1.c 
md5.c 
crypt.c 
coreprotocol.cpp 
logintask.cpp 
libyahoo.c 
yahoo_fn.c 
listtask.cpp 
statusnotifiertask.cpp 
mailnotifiertask.cpp 
messagereceivertask.cpp 
sendnotifytask.cpp 
sendmessagetask.cpp 
logofftask.cpp 
changestatustask.cpp 
modifybuddytask.cpp 
picturenotifiertask.cpp 
requestpicturetask.cpp 
yahoobuddyiconloader.cpp 
stealthtask.cpp 
sendpicturetask.cpp 
webcamimgformat.cpp
webcamtask.cpp 
conferencetask.cpp 
sendauthresptask.cpp 
pingtask.cpp
alivetask.cpp
yabtask.cpp 
yabentry.cpp 
modifyyabtask.cpp 
chatsessiontask.cpp 
filetransfernotifiertask.cpp 
sendfiletask.cpp 
receivefiletask.cpp 
yahoochattask.cpp
)


kde4_add_library( kyahoo SHARED ${libkyahoo_SRCS} )

target_link_libraries( kyahoo ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${JASPER_LIBRARIES} )
set_target_properties( kyahoo PROPERTIES VERSION 1.0.0 SOVERSION 1 )


install(TARGETS kyahoo ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############





