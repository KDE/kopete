libcricketbase_la_SOURCES = socketaddress.cc \
                            time.cc \
                            asyncudpsocket.cc \
                            messagequeue.cc \
                            thread.cc \
                            physicalsocketserver.cc \
                            bytebuffer.cc \
                            asyncpacketsocket.cc \
                            network.cc \
                            asynctcpsocket.cc \
                            socketadapters.cc \
                            md5c.c \
                            base64.cc \
                            task.cc \
                            taskrunner.cc \
			    host.cc \
			    socketaddresspair.cc 

noinst_HEADERS =            asyncfile.h \
                            common.h \
			    asyncpacketsocket.h \
		  	    socketfactory.h \
			    asyncsocket.h \
	       		    socket.h \
                            asynctcpsocket.h \
		     	    linked_ptr.h \
                            asyncudpsocket.h \
		     	    logging.h \
	      		    socketserver.h \
                            base64.h \
	     		    md5.h \
	  		    stl_decl.h \
                            basicdefs.h \
		 	    messagequeue.h \
                            basictypes.h \
			    stringutils.h \
                            bytebuffer.h \
		       	    task.h \
                            byteorder.h \
	       		    taskrunner.h \
                            criticalsection.h \
		      	    network.h \
	      		    thread.h \
		     	    time.h \
			    physicalsocketserver.h \
	     		    proxyinfo.h \
                            host.h \
	   		    scoped_ptr.h \
		 	    sigslot.h \
	      		    winping.h \
		 	    socketadapters.h \
		     	    socketaddress.h \
			    host.h \
			    socketaddresspair.h

AM_CXXFLAGS = -DPOSIX 
noinst_LTLIBRARIES = libcricketbase.la
DEFAULT_INCLUDES = -I$(srcdir)/../..
