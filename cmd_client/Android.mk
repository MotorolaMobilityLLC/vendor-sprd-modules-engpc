# Copyright 2015 The Android Open Source Project

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_SRC_FILES := engpc_cmd.c \
    engpc_cmd_parser.c \

LOCAL_MODULE := factory_cli
LOCAL_MODULE_TAGS := optional
LOCAL_MULTILIB := 32
LOCAL_PROPRIETARY_MODULE := true

LOCAL_CFLAGS := -D__STDC_LIMIT_MACROS -Werror

LOCAL_SHARED_LIBRARIES := \
    libcutils \
    liblog \
    libbase \
    libutils \

#SPRD_FACTORY_CLI_FILE := /vendor/bin/factory_cli
#SYMLINK := /vendor/bin/factory
#LOCAL_POST_INSTALL_CMD := $(hide) \
#    rm -rf $(SYMLINK) ;\
#    ln -sf $(SPRD_FACTORY_CLI_FILE) $(SYMLINK);

LOCAL_REQUIRED_MODULES := ENG_CMD.conf

include $(BUILD_EXECUTABLE)


include $(CLEAR_VARS)
TARGET_MODULE_OUT := $(PRODUCT_OUT)/vendor/etc
LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := ENG_CMD.conf
LOCAL_MODULE_CLASS := ETC
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(LOCAL_MODULE)
LOCAL_MODULE_PATH := $(TARGET_MODULE_OUT)

include $(BUILD_PREBUILT)