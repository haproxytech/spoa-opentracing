## Process this file with automake to produce Makefile.in
##
AM_CPPFLAGS = $(SPOA_OPENTRACING_CPPFLAGS)
  AM_CFLAGS = $(SPOA_OPENTRACING_CFLAGS)
 AM_LDFLAGS = $(SPOA_OPENTRACING_LDFLAGS)
       LIBS = @LIBS@ @SPOA_OPENTRACING_LIBS@

spoa_opentracing_CPPFLAGS = $(AM_CPPFLAGS) -DPACKAGE_BUILD=`cat .build-counter` -I../include
  spoa_opentracing_CFLAGS = $(AM_CFLAGS)
 spoa_opentracing_LDFLAGS = $(AM_LDFLAGS)
             bin_PROGRAMS = spoa-opentracing
 spoa_opentracing_SOURCES = \
	libev.c \
	main.c \
	spoa-message.c \
	spoa.c \
	spoe-decode.c \
	spoe-encode.c \
	spoe.c \
	spop-ack.c \
	spop-disconnect.c \
	spop-hello.c \
	spop-notify.c \
	spop-unset.c \
	tcp.c \
	util.c \
	worker.c

if WANT_OPENTRACING
spoa_opentracing_SOURCES += opentracing.c opentracing-scope.c
endif

CLEANFILES = a.out

clean: clean-am build-counter

build-counter:
	$(AM_V_GEN)../scripts/build-counter
##
## Makefile.am ends here
