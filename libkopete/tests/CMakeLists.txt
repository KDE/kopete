
if(KDE4_BUILD_TESTS)

set(kopete_test_mock_SRCS
mock/kopeteaccount_mock.cpp
mock/kopetecontact_mock.cpp
mock/kopetemessage_mock.cpp
mock/kopetemetacontact_mock.cpp
mock/kopeteprotocol_mock.cpp
)

#add_subdirectory( mock )

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/mock/ ${KOPETE_INCLUDES} )

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR} )

add_definitions( -DSRCDIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/\\"" )

set( KOPETE_TEST_LIBRARIES ${QT_QTTEST_LIBRARY} ${KDE4_KIO_LIBS} kopete )

########### Automated tests ###############

set(kopeteemoticontest_SRCS kopeteemoticontest.cpp )

kde4_automoc(${kopeteemoticontest_SRCS})

kde4_add_executable(kopeteemoticontest ${kopeteemoticontest_SRCS})

target_link_libraries(kopeteemoticontest  ${KOPETE_TEST_LIBRARIES} )

add_test(kopeteemoticontest ${EXECUTABLE_OUTPUT_PATH}/kopeteemoticontest)

####

set(kopetemessage_test_SRCS kopetemessage_test.cpp ${kopete_test_mock_SRCS})

kde4_automoc(${kopetemessage_test_SRCS})

kde4_add_executable(kopetemessage_test ${kopetemessage_test_SRCS})

target_link_libraries(kopetemessage_test  ${KOPETE_TEST_LIBRARIES} )

add_test(kopetemessage_test ${EXECUTABLE_OUTPUT_PATH}/kopetemessage_test)

####

set(kopetetask_test_SRCS kopetetask_test.cpp )

kde4_automoc(${kopetetask_test_SRCS})

kde4_add_executable(kopetetask_test ${kopetetask_test_SRCS})

target_link_libraries(kopetetask_test  ${KOPETE_TEST_LIBRARIES} )

add_test(kopetetask_test ${EXECUTABLE_OUTPUT_PATH}/kopetetask_test)

####

set(statusmessage_test_SRCS statusmessage_test.cpp )

kde4_automoc(${statusmessage_test_SRCS})

kde4_add_executable(statusmessage_test ${statusmessage_test_SRCS})

target_link_libraries(statusmessage_test  ${KOPETE_TEST_LIBRARIES} )

add_test(statusmessage_test ${EXECUTABLE_OUTPUT_PATH}/statusmessage_test)

endif(KDE4_BUILD_TESTS)

########### install files ###############





#original Makefile.am contents follow:

#SUBDIRS = mock .
#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_ASCII_CAST -DQT_NO_COMPAT \
#	$(KOPETE_INCLUDES) -I$(top_srcdir)/kopete/libkopete/private -I$(top_srcdir)/kopete/libkopete/private -I$(top_srcdir)/kopete/libkopete/tests/mock $(all_includes) -DSRCDIR=\"$(top_srcdir)/kopete/libkopete/tests\"
#METASOURCES = AUTO
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
#LDADD = ../libkopete.la -lQtTest_debug mock/libkopete_mock.la
#
##check_LTLIBRARIES = kunittest_kopetemessage_test.la kunittest_kopetecontactlist_test.la
#
#check_PROGRAMS = kopeteemoticontest kopetemessage_test kopetetask_test statusmessage_test
#TESTS = kopeteemoticontest kopetemessage_test kopetetask_test statusmessage_test
#
#kopeteemoticontest_SOURCES = kopeteemoticontest.cpp
#
#kopetemessage_test_SOURCES = kopetemessage_test.cpp
#
#kopetetask_test_SOURCES = kopetetask_test.cpp
#
#statusmessage_test_SOURCES = statusmessage_test.cpp
#
##check_PROGRAMS = kopetewallettest_program kopetepasswordtest_program
#
##kunittest_kopetepropertiestest_la_SOURCES = kopetepropertiestest.cpp ../kopeteproperties.cpp
##kunittest_kopetepropertiestest_la_LIBADD = -lkunittest ../libkopete.la
##kunittest_kopetepropertiestest_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
##kunittest_kopeteemoticontest_la_SOURCES = kopeteemoticontest.cpp
##kunittest_kopeteemoticontest_la_LIBADD = -lkunittest ../libkopete.la
##kunittest_kopeteemoticontest_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
##kunittest_kopetemessage_test_la_SOURCES = kopetemessage_test.cpp
##kunittest_kopetemessage_test_la_LIBADD = -lkunittest mock/libkopete_mock.la
##kunittest_kopetemessage_test_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
##kopetewallettest_program_SOURCES = kopetewallettest_program.cpp
##kopetewallettest_program_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
##kopetewallettest_program_LDADD = ../libkopete.la
#
##kopetepasswordtest_program_SOURCES = kopetepasswordtest_program.cpp
##kopetepasswordtest_program_LDFLAGS = -no-undefined $(all_libraries) $(KDE_RPATH)
##kopetepasswordtest_program_LDADD = ../libkopete.la
#
##kunittest_kopetecontactlist_test_la_SOURCES = kopetecontactlist_test.cpp
##kunittest_kopetecontactlist_test_la_LIBADD = -lkunittest mock/libkopete_mock.la
##kunittest_kopetecontactlist_test_la_LDFLAGS = -module $(KDE_CHECK_PLUGIN) $(all_libraries)
#
##noinst_HEADERS = kopetepropertiestest.h kopeteemoticontest.h
#
##check-local: 
##	kunittestmodrunner
##guicheck:
##	kunittestmod $(PWD)
#
