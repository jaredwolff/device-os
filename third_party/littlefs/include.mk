TARGET_LITTLEFS_PATH = $(LITTLEFS_MODULE_PATH)
INCLUDE_DIRS += $(LITTLEFS_MODULE_PATH)/littlefs

CFLAGS += -DLFS_CONFIG=lfs_config.h
