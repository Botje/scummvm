MODULE := engines/kq8

MODULE_OBJS = \
	bitmap.o \
	font.o \
	gfx_base.o \
	gfx_opengls.o \
	kq8.o \
	console.o \
	graphics_manager.o \
	main_screen.o \
	palette.o \
	script.o \
	texture_packer.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_KQ8), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
