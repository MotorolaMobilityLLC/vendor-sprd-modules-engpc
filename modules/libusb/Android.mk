LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := usbtypec.c

LOCAL_MODULE := libusbtypec
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := npidevice
LOCAL_PROPRIETARY_MODULE := true

LOCAL_C_INCLUDES:= $(TOP)/vendor/sprd/proprietories-source/engpc/sprd_fts_inc hardware/libhardware/include/hardware

LOCAL_SHARED_LIBRARIES:= liblog libc libcutils libdl libutils libbase
include $(BUILD_SHARED_LIBRARY)
