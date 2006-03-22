kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(challengetest_SRCS challengetest.cpp )

kde4_automoc(${challengetest_SRCS})

if(KDE4_BUILD_TESTS)

kde4_add_executable(challengetest ${challengetest_SRCS})

target_link_libraries(challengetest  ${KDE4_KDECORE_LIBS} )

endif(KDE4_BUILD_TESTS)

########### install files ###############


kde4_footer()



#original Makefile.am contents follow:

#AM_CPPFLAGS = -DKDE_NO_COMPAT -DQT_NO_ASCII_CAST -DQT_NO_COMPAT \
#	$(KOPETE_INCLUDES) $(all_includes) -DSRCDIR=\"$(top_srcdir)/kopete/protocols/msn/tests\" \
#	-I./..
#
#METASOURCES = AUTO
#
#AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)
#LDADD = -lQtTest_debug ../libkopete_msn_shared.la 
#
#check_PROGRAMS = challengetest
#TESTS = challengetest
#
#challengetest_SOURCES = challengetest.cpp
