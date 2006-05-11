kde4_header()

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
webcamtask.cpp 
conferencetask.cpp 
sendauthresptask.cpp 
pingtask.cpp 
yabtask.cpp 
yabentry.cpp 
modifyyabtask.cpp 
chatsessiontask.cpp 
)

kde4_automoc( ${libkyahoo_SRCS} )

kde4_add_library( kyahoo SHARED ${libkyahoo_SRCS} )

target_link_libraries( kyahoo ${KDE4_KIO_LIBS} ${QT_QTXML_LIBRARY} ${QT_QTNETWORK_LIBRARY} )

install_targets( ${LIB_INSTALL_DIR} kyahoo )

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#SUBDIRS = . tests
#METASOURCES = AUTO
#noinst_LTLIBRARIES = libkyahoo.la
#
#AM_CPPFLAGS = $(all_includes)
#
#
#libkyahoo_la_SOURCES = client.cpp task.cpp connector.cpp inputprotocolbase.cpp \
#	ymsgprotocol.cpp ymsgtransfer.cpp transfer.cpp yahoobytestream.cpp bytestream.cpp \
#	yahooclientstream.cpp yahooconnector.cpp safedelete.cpp stream.cpp sha1.c md5.c crypt.c \
#	coreprotocol.cpp logintask.cpp libyahoo.c yahoo_fn.c listtask.cpp statusnotifiertask.cpp \
#	mailnotifiertask.cpp messagereceivertask.cpp sendnotifytask.cpp sendmessagetask.cpp \
#	logofftask.cpp changestatustask.cpp modifybuddytask.cpp picturenotifiertask.cpp \
#	requestpicturetask.cpp yahoobuddyiconloader.cpp stealthtask.cpp sendpicturetask.cpp \
#	webcamtask.cpp conferencetask.cpp sendauthresptask.cpp pingtask.cpp yabtask.cpp \
#	yabentry.cpp modifyyabtask.cpp chatsessiontask.cpp
#libkyahoo_la_LDFLAGS = -no-undefined $(all_libraries)
#libkyahoo_la_LIBADD = $(LIB_QT)
#
#noinst_HEADERS = logintask.h yabentry.h yabtask.h modifyyabtask.h \
#	chatsessiontask.h
