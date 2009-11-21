
########### next target ###############

if ( BUILD_libjingle_cricketbase )

SET(cricketbase_STAT_SRCS
   socketaddress.cc
   time.cc
   asyncudpsocket.cc
   messagequeue.cc
   thread.cc
   physicalsocketserver.cc
   bytebuffer.cc
   asyncpacketsocket.cc
   network.cc
   asynctcpsocket.cc
   socketadapters.cc
   md5c.c
   base64.cc
   task.cc
   taskrunner.cc
   host.cc
   proxydetect.cc
   socketaddresspair.cc
   stringencode.cc
   stringdigest.cc
   stringutils.cc
   proxyinfo.cc
   common.cc
   logging.cc
   stream.cc
   ssladapter.cc
   openssladapter.cc
   helpers.cc
   asynchttprequest.cc
   firewallsocketserver.cc
   httpcommon.cc
   httpbase.cc
   httpclient.cc
   httpserver.cc
   socketpool.cc
   signalthread.cc
   autodetectproxy.cc
   urlencode.cc
   pathutils.cc
   fileutils.cc
   unixfilesystem.cc
   tarstream.cc
   streamutils.cc
   diskcache.cc
   diskcachestd.cc
)

add_library(cricketbase STATIC ${cricketbase_STAT_SRCS})

endif ( BUILD_libjingle_cricketbase )


########### next target ###############

if ( BUILD_libjingle_crickettest )

SET(crickettest_STAT_SRCS
   testclient.cc
   natserver.cc
   natsocketfactory.cc
   nattypes.cc
   virtualsocketserver.cc
)

add_library(crickettest STATIC ${crickettest_STAT_SRCS})

endif ( BUILD_libjingle_crickettest )


########### next target ###############

if ( BUILD_libjingle_natserver )

SET(natserver_SRCS
   natserver_main.cc
)

add_executable(natserver ${natserver_SRCS})

target_link_libraries(natserver crickettest cricketbase pthread)

endif ( BUILD_libjingle_natserver )


########### next target ###############

if ( BUILD_libjingle_nat_unittest )

SET(nat_unittest_SRCS
   nat_unittest.cc
)

add_executable(nat_unittest ${nat_unittest_SRCS})

target_link_libraries(nat_unittest crickettest cricketbase pthread)

endif ( BUILD_libjingle_nat_unittest )


########### next target ###############

if ( BUILD_libjingle_virtualsocket_unittest )

SET(virtualsocket_unittest_SRCS
   virtualsocket_unittest.cc
)

add_executable(virtualsocket_unittest ${virtualsocket_unittest_SRCS})

target_link_libraries(virtualsocket_unittest crickettest cricketbase pthread)

endif ( BUILD_libjingle_virtualsocket_unittest )


########### install files ###############

