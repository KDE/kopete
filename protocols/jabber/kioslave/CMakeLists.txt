kde4_header()

include_directories( ${KDE4_INCLUDE_DIR} ${QT_INCLUDES} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}  )


########### next target ###############

set(kio_jabberdisco_PART_SRCS jabberdisco.cpp )

kde4_automoc(${kio_jabberdisco_PART_SRCS})

kde4_add_plugin(kio_jabberdisco ${kio_jabberdisco_PART_SRCS})

kde4_install_libtool_file( ${PLUGIN_INSTALL_DIR} kio_jabberdisco )

target_link_libraries(kio_jabberdisco  ${KDE4_KDECORE_LIBS} jabberclient iris_local kio )

install_targets(${LIB_INSTALL_DIR}/kde4 kio_jabberdisco )


########### install files ###############

install_files( ${SERVICES_INSTALL_DIR} FILES jabberdisco.protocol )

kde4_footer()



#original Makefile.am contents follow:

#METASOURCES = AUTO
#
#INCLUDES = \
#	-I$(srcdir)/.. \
#	-I$(srcdir)/../libiris/iris/include \
#	-I$(srcdir)/../libiris/iris/xmpp-im \
#	-I$(srcdir)/../libiris/iris/jabber \
#	-I$(srcdir)/../libiris/qca/src \
#	-I$(srcdir)/../libiris/cutestuff/util \
#	-I$(srcdir)/../libiris/cutestuff/network \
#	$(all_includes)
#                                                        
#kde_module_LTLIBRARIES = kio_jabberdisco.la
#
#kio_jabberdisco_la_SOURCES = jabberdisco.cpp
#kio_jabberdisco_la_LIBADD = ../libjabberclient.la ../libiris/libiris_local.la $(LIB_KIO)
#kio_jabberdisco_la_LDFLAGS = -no-undefined -module $(KDE_PLUGIN) $(all_libraries)
#
#noinst_HEADERS = jabberdisco.h
#
#protocol_DATA = jabberdisco.protocol
#protocoldir = $(kde_servicesdir)
#
#messages: rc.cpp
#	$(XGETTEXT) *.cpp -o $(podir)/kio_jabberdisco.pot
