
if include_motionautoaway
MOTIONAUTOAWAY_SUBDIR=motionautoaway
endif

if dont_have_kopete_plugins_opt
SUBDIRS = autoreplace history contactnotes cryptography\
                connectionstatus translator nowlistening webpresence texteffect\
		highlight $(MOTIONAUTOAWAY_SUBDIR)
endif

if have_kopete_plugins_opt
SUBDIRS = $(KOPETE_PLUGINS_SUBDIRS)
endif
