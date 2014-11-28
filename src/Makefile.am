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
	       server.c

delete_con_SOURCES=delete_con.c

btserver_LDADD=-lpthread -lsybdb

delete_con_LDADD=-lsybdb

noinst_HEADERS=head.h freetdstodb.h

<<<<<<< HEAD
INCLUDES=-I$(top_srcdir)include -I$(top_srcdir)/CONFIG -I$(top_srcdir)/DES -I$(top_srcdir)/COMPRESS

btserver_LDADD+=$(top_builddir)/CONFIG/libconfig.la \
	     $(top_builddir)/DES/libdes.la \
	     $(top_builddir)/COMPRESS/libgz.la
=======
INCLUDES=-I$(top_srcdir)include -I$(top_srcdir)/CONFIG -I$(top_srcdir)/DES

btserver_LDADD+=$(top_builddir)/CONFIG/libconfig.la \
	     $(top_builddir)/DES/libdes.la
>>>>>>> origin/master

delete_con_LDADD+=$(top_builddir)/CONFIG/libconfig.la
