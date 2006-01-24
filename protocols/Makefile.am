if include_meanwhile
MEANWHILE=meanwhile
endif

if include_gadu
GADU=gadu
endif

if include_jabber
JABBER=jabber
endif

if include_testbed
TESTBED=testbed
endif

if include_smsgsm
SMS=sms
endif

#SUBDIRS = $(TESTBED) groupwise msn irc oscar yahoo winpopup $(SMS) $(JABBER) $(GADU) $(MEANWHILE)

# Disabled Jabber, IRC, Groupwise, SMS and Meanwhile for compiling, because they are not ported right now
# I leave that to the protocol maintainer (-DarkShock)
SUBDIRS = $(TESTBED) msn oscar yahoo winpopup $(GADU)
