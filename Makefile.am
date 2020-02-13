AM_MAKEFLAGS = --no-print-directory

if EXTERNAL_ELL
ell_cflags = @ELL_CFLAGS@
ell_ldadd = @ELL_LIBS@
ell_dependencies =
ell_built_sources =
else
ell_cflags =
ell_ldadd = ell/libell-internal.la
ell_dependencies = $(ell_ldadd)
ell_built_sources = ell/internal ell/ell.h

noinst_LTLIBRARIES = ell/libell-internal.la

ell_headers = ell/util.h \
			ell/test.h \
			ell/strv.h \
			ell/utf8.h \
			ell/queue.h \
			ell/hashmap.h \
			ell/string.h \
			ell/settings.h \
			ell/main.h \
			ell/idle.h \
			ell/signal.h \
			ell/timeout.h \
			ell/io.h \
			ell/ringbuf.h \
			ell/log.h \
			ell/plugin.h \
			ell/checksum.h \
			ell/netlink.h \
			ell/genl.h \
			ell/dbus.h \
			ell/dbus-service.h \
			ell/dbus-client.h \
			ell/hwdb.h \
			ell/cipher.h \
			ell/random.h \
			ell/uintset.h \
			ell/base64.h \
			ell/pem.h \
			ell/tls.h \
			ell/uuid.h \
			ell/key.h \
			ell/pkcs5.h \
			ell/file.h \
			ell/dir.h \
			ell/net.h \
			ell/dhcp.h \
			ell/cert.h \
			ell/ecc.h \
			ell/ecdh.h \
			ell/time.h \
			ell/path.h

ell_sources = ell/private.h \
			ell/missing.h \
			ell/util.c \
			ell/test.c \
			ell/strv.c \
			ell/utf8.c \
			ell/queue.c \
			ell/hashmap.c \
			ell/string.c \
			ell/settings.c \
			ell/main.c \
			ell/idle.c \
			ell/signal.c \
			ell/timeout.c \
			ell/io.c \
			ell/ringbuf.c \
			ell/log.c \
			ell/plugin.c \
			ell/checksum.c \
			ell/netlink-private.h \
			ell/netlink.c \
			ell/genl-private.h \
			ell/genl.c \
			ell/dbus-private.h \
			ell/dbus.c \
			ell/dbus-message.c \
			ell/dbus-util.c \
			ell/dbus-service.c \
			ell/dbus-client.c \
			ell/dbus-name-cache.c \
			ell/dbus-filter.c \
			ell/gvariant-private.h \
			ell/gvariant-util.c \
			ell/siphash-private.h \
			ell/siphash.c \
			ell/hwdb.c \
			ell/cipher.c \
			ell/random.c \
			ell/uintset.c \
			ell/base64.c \
			ell/asn1-private.h \
			ell/pem.c \
			ell/pem-private.h \
			ell/tls-private.h \
			ell/tls.c \
			ell/tls-record.c \
			ell/tls-suites.c \
			ell/tls-extensions.c \
			ell/uuid.c \
			ell/key.c \
			ell/pkcs5-private.h \
			ell/pkcs5.c \
			ell/file.c \
			ell/dir.c \
			ell/net.c \
			ell/dhcp-private.h \
			ell/dhcp.c \
			ell/dhcp-transport.c \
			ell/dhcp-lease.c \
			ell/cert-private.h \
			ell/cert.c \
			ell/ecc-external.c \
			ell/ecc-private.h \
			ell/ecc.c \
			ell/ecdh.c \
			ell/time.c \
			ell/path.c

ell_libell_internal_la_SOURCES = $(ell_headers) $(ell_sources)
endif

bin_PROGRAMS = src/connmand2

src_connmand2_SOURCES = src/main.c
src_connmand2_LDADD = $(ell_ldadd) -ldl
src_connmand2_DEPENDENCIES = $(ell_dependencies)

AM_CFLAGS = $(ell_cflags) -fvisibility=hidden

DISTCLEANFILES = $(BUILT_SOURCES)

MAINTAINERCLEANFILES = Makefile.in configure config.h.in aclocal.m4

BUILT_SOURCES = $(ell_built_sources)

ell/internal: Makefile
	$(AM_V_at)$(MKDIR_P) ell
	$(AM_V_GEN)for f in $(ell_headers) $(ell_sources) ; do \
		if [ ! -f $$f ] ; then \
			$(LN_S) -t ell -f $(abs_srcdir)/../ell/$$f ; \
		fi \
	done > $@

ell/ell.h: Makefile
	$(AM_V_at)echo -n > $@
	$(AM_V_GEN)for f in $(ell_headers) ; do \
		echo "#include <$$f>" >> $@ ; \
	done

maintainer-clean-local:
	-rm -rf build-aux ell
