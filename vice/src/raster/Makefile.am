AM_CPPFLAGS = \
	@VICE_CPPFLAGS@ \
	@ARCH_INCLUDES@ \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/joyport

AM_CFLAGS = @VICE_CFLAGS@

AM_CXXFLAGS = @VICE_CXXFLAGS@

AM_LDFLAGS = @VICE_LDFLAGS@


noinst_LIBRARIES = libraster.a

libraster_a_SOURCES = \
	raster-cache-const.h \
	raster-cache-fill-1fff.h \
	raster-cache-fill-39ff.h \
	raster-cache-fill.h \
	raster-cache-nibbles.h \
	raster-cache-text-ext.h \
	raster-cache-text-std.h \
	raster-cache.c \
	raster-cache.h \
	raster-canvas.c \
	raster-canvas.h \
	raster-changes.c \
	raster-changes.h \
	raster-cmdline-options.c \
	raster-cmdline-options.h \
	raster-line-changes-sprite.c \
	raster-line-changes.c \
	raster-line.c \
	raster-line.h \
	raster-modes.c \
	raster-modes.h \
	raster-resources.c \
	raster-resources.h \
	raster-snapshot.c \
	raster-snapshot.h \
	raster-sprite.c \
	raster-sprite.h \
	raster-sprite-status.c \
	raster-sprite-status.h \
	raster-sprite-cache.c \
	raster-sprite-cache.h \
	raster.c \
	raster.h

