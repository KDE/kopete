# INCLUDES = @GLIB_CFLAGS@
AM_CPPFLAGS = $(all_includes) $(GLIB_CFLAGS)

# libmimicincludedir = $(includedir)
# libmimicinclude_HEADERS = mimic.h

noinst_LTLIBRARIES = libmimic.la
libmimic_la_SOURCES = \
	mimic.c \
	encode.c \
	decode.c \
	bitstring.c \
	vlc_common.c \
	vlc_encode.c \
	vlc_decode.c \
	fdct_quant.c \
	idct_dequant.c \
	colorspace.c \
	deblock.c \
	mimic-private.h
# libmimic_la_LDFLAGS = \
# 	-version-info $(MIMIC_CURRENT):$(MIMIC_REVISION):$(MIMIC_AGE) \
# 	-export-symbols-regex "^[^_].*"

