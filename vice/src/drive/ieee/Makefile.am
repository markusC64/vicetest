AM_CPPFLAGS = \
	@VICE_CPPFLAGS@ \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/core/rtc

AM_CFLAGS = @VICE_CFLAGS@

AM_CXXFLAGS = @VICE_CXXFLAGS@

AM_LDFLAGS = @VICE_LDFLAGS@


noinst_LIBRARIES = libdriveieee.a

libdriveieee_a_SOURCES = \
	fdc.c \
	fdc.h \
	ieee-cmdline-options.c \
	ieee-cmdline-options.h \
	ieee-resources.c \
	ieee-resources.h \
	ieee.c \
	ieeerom.c \
	ieeerom.h \
	memieee.c \
	memieee.h \
	riot1d.c \
	riot2d.c \
	riotd.h \
	via1d2031.h \
	via1d2031.c
