LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_32_BIT_ONLY := true

LOCAL_SRC_FILES := dcx_cali.c

LOCAL_MODULE := libdcxcali
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := npidevice
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES:= \
    $(TOP)/vendor/sprd/proprietories-source/engmode

LOCAL_SHARED_LIBRARIES:= liblog libc libcutils

include $(BUILD_SHARED_LIBRARY)
