LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_32_BIT_ONLY := true

LOCAL_SRC_FILES := charge.c

LOCAL_MODULE := libcharge
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := npidevice

LOCAL_C_INCLUDES:= \
	$(TOP)/vendor/sprd/proprietories-source/engmode \
	$(TOP)/vendor/sprd/proprietories-source/engmode/charge

LOCAL_SHARED_LIBRARIES:= liblog libc libcutils

LOCAL_PROPRIETARY_MODULE := true
include $(BUILD_SHARED_LIBRARY)
