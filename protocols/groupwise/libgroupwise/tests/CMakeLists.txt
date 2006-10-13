
include_directories( ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise/qca/src ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise/libgroupwise ${CMAKE_SOURCE_DIR}/kopete/protocols/groupwise  )


########### next target ###############

set(clientstream_test_SRCS clientstream_test.cpp )

kde4_automoc(clientstream_test ${clientstream_test_SRCS})

kde4_add_executable(clientstream_test ${clientstream_test_SRCS})

target_link_libraries(clientstream_test  ${KDE4_KDECORE_LIBS} qt-mt )


########### next target ###############

set(field_test_SRCS field_test.cpp )

kde4_automoc(field_test ${field_test_SRCS})

kde4_add_executable(field_test ${field_test_SRCS})

target_link_libraries(field_test  ${KDE4_KDECORE_LIBS} gwtest qt-mt )


########### next target ###############

set(coreprotocol_test_SRCS coreprotocol_test.cpp )

kde4_automoc(coreprotocol_test ${coreprotocol_test_SRCS})

kde4_add_executable(coreprotocol_test ${coreprotocol_test_SRCS})

target_link_libraries(coreprotocol_test  ${KDE4_KDECORE_LIBS} gwtest qt-mt )


########### next target ###############

set(client_test_SRCS client_test.cpp )

kde4_automoc(client_test ${client_test_SRCS})

kde4_add_executable(client_test ${client_test_SRCS})

target_link_libraries(client_test  ${KDE4_KDECORE_LIBS} gwtest qt-mt )


########### install files ###############





#original Makefile.am contents follow:

#INCLUDES = -I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise/qca/src \
#	-I$(top_srcdir)/kopete/protocols/groupwise/libgroupwise \
#	-I$(top_srcdir)/kopete/protocols/groupwise \
#	$(all_includes)
#METASOURCES = AUTO
#noinst_PROGRAMS = clientstream_test field_test coreprotocol_test client_test
#coreprotocol_test_LDFLAGS = -no-undefined $(all_libraries)
#coreprotocol_test_SOURCES = coreprotocol_test.cpp
#coreprotocol_test_LDADD = \
#	../libgwtest.la -lqt-mt
#field_test_LDFLAGS = -no-undefined $(all_libraries)
#field_test_SOURCES = field_test.cpp
#field_test_LDADD = \
#	../libgwtest.la -lqt-mt
#
#clientstream_test_SOURCES = clientstream_test.cpp
#clientstream_test_LDADD = -lqt-mt \
#	../../kopete_groupwise.la
#
#client_test_SOURCES = client_test.cpp
#client_test_LDADD = ../../../../protocols/groupwise/kopete_groupwise.la \
#	../libgwtest.la -lqt-mt
