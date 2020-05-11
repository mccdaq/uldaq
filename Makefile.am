AUTOMAKE_OPTIONS = dist-bzip2 no-dist-gzip subdir-objects
ACLOCAL_AMFLAGS = -I m4
DISTCLEANFILES = libuldaq.pc

SUBDIRS = src

if BUILD_EXAMPLES
SUBDIRS += examples
endif

EXTRA_DIST = doc/Doxyfile doc/DoxygenLayout.xml doc/pagesref.css doc/pagesref.txt

dist_doc_DATA = README.md

pkgconfigdir=$(libdir)/pkgconfig
dist_pkgconfig_DATA=libuldaq.pc

# fpgadatadir=/etc/uldaq/fpga
# dist_fpgadata_DATA=fpga/USB_1208HS.rbf\
# fpga/USB_1608G_2.rbf\
# fpga/USB_1608G.rbf\
# fpga/USB_1808.bin\
# fpga/usb_2020.bin\
# fpga/USB_26xx.rbf\
# fpga/USB_CTR.bin\
# fpga/USB_DIO32HS.bin

if OS_LINUX
rulesdatadir=/lib/udev/rules.d/
dist_rulesdata_DATA=rules/50-uldaq.rules
endif

dist-hook:
	chmod u+w $(distdir)/examples/*.c
	chmod u+w $(distdir)/examples/*.h

.PHONY: dist-up

reldir = .release/$(distdir)
dist-up: dist
	rm -rf $(reldir)
	mkdir -p $(reldir)
	cp $(distdir).tar.bz2 $(reldir)

if OS_LINUX
install-data-hook:
	udevadm control --reload-rules && \
	test -f "/etc/redhat-release" && echo "/usr/local/lib" > /etc/ld.so.conf.d/uldaq.conf || echo "" && \
	test -f "/etc/arch-release" && echo "/usr/local/lib" > /etc/ld.so.conf.d/uldaq.conf || echo "" && \
	ldconfig
endif

if OS_LINUX
uninstall-hook:
	test -f "/etc/ld.so.conf.d/uldaq.conf" &&  rm "/etc/ld.so.conf.d/uldaq.conf" || echo ""
endif

