AUTOMAKE_OPTIONS=foreign

bin_PROGRAMS=btserver delete_con

btserver_SOURCES=cgi_business.c \
	       cgi_ctrl.c \
	       dbctrl.c \
	       freetdstodb.c \
	       mi_business.c \
	       net.c \
	       protocol.c \
	       userlink.c \
	       log.c \
	       curl_ctl.c \
	       dpm_user.c \
	       encode_and_decode.c \
	       json_analysis.c \
	       webctrl.c \
	       compress.c \
	       md5.c \
	       sync.c \
	       server.c

delete_con_SOURCES=delete_con.c

btserver_LDADD=-lpthread -lsybdb -lcurl -ljson-c -lz -levent

delete_con_LDADD=-lsybdb

noinst_HEADERS=head.h freetdstodb.h json_analysis.h HMACSHA1.h log.h encode_and_decode.h curl_ctl.h dpm_user.h sha1.h

INCLUDES=-I$(top_srcdir)/CONFIG -I$(top_srcdir)/DES

btserver_LDADD+=$(top_builddir)/CONFIG/libconfig.la \
	     $(top_builddir)/DES/libdes.la

delete_con_LDADD+=$(top_builddir)/CONFIG/libconfig.la
