kde4_header()

add_subdirectory( tests ) 

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


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
