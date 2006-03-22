kde4_header()

include_directories( ${CMAKE_SOURCE_DIR}/kopete/protocols/oscar/liboscar ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(buffertest_SRCS buffertest.cpp )

kde4_automoc(${buffertest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(buffertest ${buffertest_SRCS})

target_link_libraries(buffertest  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(serverversionstest_SRCS serverversionstest.cpp oscartestbase.cpp )

kde4_automoc(${serverversionstest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(serverversionstest ${serverversionstest_SRCS})

target_link_libraries(serverversionstest  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(connectiontest_SRCS connectiontest.cpp oscartestbase.cpp )

kde4_automoc(${connectiontest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(connectiontest ${connectiontest_SRCS})

target_link_libraries(connectiontest  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(rateinfotest_SRCS rateinfotest.cpp oscartestbase.cpp )

kde4_automoc(${rateinfotest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(rateinfotest ${rateinfotest_SRCS})

target_link_libraries(rateinfotest  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### next target ###############

set(offlinemessagestest_SRCS offlinemessagestest.cpp oscartestbase.cpp )

kde4_automoc(${offlinemessagestest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(offlinemessagestest ${offlinemessagestest_SRCS})

target_link_libraries(offlinemessagestest  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#INCLUDES = $(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/protocols/oscar/liboscar $(all_includes)
#METASOURCES = AUTO
#check_PROGRAMS = buffertest serverversionstest connectiontest rateinfotest offlinemessagestest
#TESTS = buffertest serverversionstest connectiontest rateinfotest offlinemessagestest
#AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)
#
#LDADD = ../liboscar.la -lQtTest_debug
#
#buffertest_SOURCES = buffertest.cpp
#
#serverversionstest_SOURCES = serverversionstest.cpp oscartestbase.cpp
#connectiontest_SOURCES = connectiontest.cpp oscartestbase.cpp
#rateinfotest_SOURCES = rateinfotest.cpp oscartestbase.cpp
#offlinemessagestest_SOURCES = offlinemessagestest.cpp oscartestbase.cpp
#
#
