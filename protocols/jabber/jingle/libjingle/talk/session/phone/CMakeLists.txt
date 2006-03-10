kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############


########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#libcricketsessionphone_la_SOURCES = audiomonitor.cc \
#				    channelmanager.cc \
#				    voicechannel.cc \
#				    call.cc \
#				    phonesessionclient.cc \
#				    linphonemediaengine.cc 
#
#noinst_HEADERS =                    audiomonitor.h \
#				    channelmanager.h \
#				    linphonemediaengine.h \
#				    mediaengine.h \
#				    phonesessionclient.h \
#				    voicechannel.h \
#                                    call.h \
#				    mediachannel.h 
#
#AM_CPPFLAGS = -DPOSIX $(ORTP_CFLAGS) $(ILBC_CFLAGS) -I$(srcdir)/../../../talk/third_party/mediastreamer -I$(srcdir)/../../.. $(GLIB_CFLAGS) $(SPEEX_CFLAGS)
#noinst_LTLIBRARIES = libcricketsessionphone.la
