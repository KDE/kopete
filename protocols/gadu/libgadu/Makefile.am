METASOURCES = AUTO
noinst_LTLIBRARIES = libgadu_copy.la
INCLUDES = $(SSL_INCLUDES) 
libgadu_copy_la_LDFLAGS = $(SSL_LDFLAGS)
libgadu_copy_la_LIBADD =  $(LIBSSL) $(LIBPTHREAD) 
libgadu_copy_la_SOURCES = common.c \
			  dcc.c \
			  states.c \
			  events.c \
			  http.c \
			  libgadu.c \
			  pubdir50.c \
			  pubdir.c 
