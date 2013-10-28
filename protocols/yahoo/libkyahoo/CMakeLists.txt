include_directories( ${JASPER_INCLUDE_DIR} )

add_subdirectory( tests ) 

include(CheckIncludeFile)
check_include_file(stdint.h HAVE_STDINT_H)
check_include_file(inttypes.h HAVE_INTTYPES_H)
if(HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif(HAVE_STDINT_H)
if(HAVE_INTTYPES_H)
    add_definitions(-DHAVE_INTTYPES_H)
endif(HAVE_INTTYPES_H)

include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)
if(WORDS_BIGENDIAN)
    add_definitions(-DWORDS_BIGENDIAN)
endif(WORDS_BIGENDIAN)

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

if(MINGW)
    set_source_files_properties(yahoo_fn.c PROPERTIES COMPILE_FLAGS "--std=c99")
endif(MINGW)

kde4_add_library( kyahoo SHARED ${libkyahoo_SRCS} )

target_link_libraries( kyahoo ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${JASPER_LIBRARIES} )
set_target_properties( kyahoo PROPERTIES VERSION 1.0.0 SOVERSION 1 )


install(TARGETS kyahoo ${INSTALL_TARGETS_DEFAULT_ARGS})

########### install files ###############





