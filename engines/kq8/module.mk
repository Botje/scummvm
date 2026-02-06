MODULE := engines/kq8

MODULE_OBJS = \
	animation_loop_list.o \
	animation_sequence.o \
	bitmap.o \
	font.o \
	gfx_base.o \
	gfx_opengls.o \
	kq_file.o \
	kq8.o \
	console.o \
	graphics_manager.o \
	main_screen.o \
	material_file.o \
	msg_file.o \
	objects/anim_object.o \
	objects/connor.o \
	objects/door.o \
	objects/camera.o \
	objects/interior.o \
	objects/monster.o \
	objects/object.o \
	objects/object_factory.o \
	objects/terrain.o \
	objects/world_item.o \
	palette.o \
	shape.o \
	script.o \
	singletons/inventory_item_type_list.o \
	singletons/monster_manager.o \
	singletons/monster_type_list.o \
	texture_packer.o \
	world.o \
	metaengine.o

# This module can be built as a plugin
ifeq ($(ENABLE_KQ8), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
