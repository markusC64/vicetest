## Makefile.am for the native GTK3 version of `libwidgetsbase.a'.


AM_CPPFLAGS = \
	@VICE_CPPFLAGS@ \
	@ARCH_INCLUDES@ \
	-I$(top_srcdir)/src/arch/shared \
	-I$(top_builddir)/src \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/drive \
	-I$(top_srcdir)/src/joyport \
	-I$(top_srcdir)/src/lib/p64 \
	-I$(top_srcdir)/src/platform \
	-I$(top_srcdir)/src/raster \
	-I$(top_srcdir)/src/rs232drv \
	-I$(top_srcdir)/src/sid \
	-I$(top_srcdir)/src/c64 \
	-I$(top_srcdir)/src/c64dtv \
	-I$(top_srcdir)/src/c128 \
	-I$(top_srcdir)/src/cbm2 \
	-I$(top_srcdir)/src/pet \
	-I$(top_srcdir)/src/plus4 \
	-I$(top_srcdir)/src/scpu64 \
	-I$(top_srcdir)/src/vic20 \
	-I$(top_srcdir)/src/userport \
	-I$(top_srcdir)/src/c64/cart

AM_CFLAGS = @VICE_CFLAGS@

AM_CXXFLAGS = @VICE_CXXFLAGS@

AM_LDFLAGS = @VICE_LDFLAGS@


noinst_LIBRARIES = libbasewidgets.a

libbasewidgets_a_SOURCES = \
	basedialogs.c \
	carthelpers.c \
	cartimagewidget.c \
	clockportdevicewidget.c \
	csshelpers.c \
	filechooserhelpers.c \
	lastdir.c \
	openfiledialog.c \
	popupmenu.c \
	resourcebrowser.c \
	resourcecheckbutton.c \
	resourcecombobox.c \
	resourceentry.c \
	resourcehelpers.c \
	resourcenumericstring.c \
	resourceradiogroup.c \
	resourcescale.c \
	resourcespinbutton.c \
	resourceswitch.c \
	savefiledialog.c \
	selectdirectorydialog.c \
	uiactionbutton.c \
	widgethelpers.c

EXTRA_DIST = \
	basedialogs.h \
	basewidget_types.h \
	basewidgets.h \
	carthelpers.h \
	cartimagewidget.h \
	clockportdevicewidget.h \
	csshelpers.h \
	filechooserhelpers.h \
	lastdir.h \
	openfiledialog.h \
	popupmenu.h \
	resourcebrowser.h \
	resourcecheckbutton.h \
	resourcecombobox.h \
	resourceentry.h \
	resourcehelpers.h \
	resourcenumericstring.h \
	resourceradiogroup.h \
	resourcescale.h \
	resourcespinbutton.h \
	resourceswitch.h \
	savefiledialog.h \
	selectdirectorydialog.h \
	uiactionbutton.h \
	vice_gtk3.h \
	vice_gtk3_settings.h \
	widgethelpers.h
