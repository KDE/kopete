SUBDIRS=talk

dist-hook:
	sed -i -f talk/sanitize.sed `find $(distdir) -type f`