SUBDIRS = libkopete kopete protocols plugins pics sounds 

api:
	doxygen kopete.api

messages: 
	$(EXTRACTRC) `find . -name \*.ui -o -name \*.rc` >> rc.cpp
	LIST=`find . -name \*.h -o -name \*.hh -o -name \*.H -o -name \*.hxx -o -name \*.hpp -o -name \*.cpp -o -name \*.cc -o -name \*.cxx -o -name \*.ecpp -o -name \*.C`; \
	if test -n "$$LIST"; then \
		$(XGETTEXT) -ki18n $$LIST -o $(podir)/kopete.pot; \
	fi
