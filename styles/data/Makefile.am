stylesdatadir = $(top_srcdir)/kopete/styles/data
stylesDATA = iChat iChat-Trans
stylesinstalldir = $(kde_datadir)/kopete/styles/data

install:
	$(INSTALL) -d $(stylesinstalldir)
	for DIR in $(stylesDATA); do \
		cp -Rf "$(stylesdatadir)/$$DIR" $(stylesinstalldir); \
	done

uninstall:
	for DIR in $(stylesDATA); do \
		rm -Rf "$(stylesinstalldir)/$$DIR" ; \
	done
