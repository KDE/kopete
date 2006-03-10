kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(connection_test_SRCS connection_test.cpp )

kde4_automoc(${connection_test_SRCS})

kde4_add_executable(connection_test ${connection_test_SRCS})

target_link_libraries(connection_test  ${QT_AND_KDECORE_LIBS} )


########### next target ###############

set(transfer_test_SRCS transfer_test.cpp )

kde4_automoc(${transfer_test_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(transfer_test ${transfer_test_SRCS})

target_link_libraries(transfer_test  ${QT_AND_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

# AM_CPPFLAGS = -DQT_NO_ASCII_CAST -DQT_NO_COMPAT $(all_includes) -I./..
#
#METASOURCES = AUTO
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS)
#LDADD = -lQtTest_debug ../libpapillon.la 
#
#noinst_PROGRAMS = connection_test
#
#check_PROGRAMS = transfer_test
#TESTS = transfer_test
#
#transfer_test_SOURCES = transfer_test.cpp
#
#connection_test_SOURCES = connection_test.cpp
