if include_gadu
GADU=gadu
endif

if include_jabber
JABBER=jabber
endif

if dont_have_kopete_protocols_opt
SUBDIRS = msn irc oscar yahoo $(JABBER) winpopup sms $(GADU) testbed
endif

if have_kopete_protocols_opt
SUBDIRS = $(KOPETE_PROTOCOLS_SUBDIRS)
endif
