LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_32_BIT_ONLY := true

LOCAL_SRC_FILES := gpio.cpp

LOCAL_PROPRIETARY_MODULE := true
LOCAL_MODULE := libgpio
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_RELATIVE_PATH := npidevice

ifneq (,$(findstring 9863, $(TARGET_PRODUCT)))
PLATFORM := sharkl3
else ifneq (,$(findstring 7731, $(TARGET_PRODUCT)))
PLATFORM := pike2
else ifneq (,$(findstring 9832, $(TARGET_PRODUCT)))
PLATFORM := sharkle
else ifneq (,$(findstring 9820, $(TARGET_PRODUCT)))
PLATFORM := sharkle
else ifneq (,$(findstring 9853, $(TARGET_PRODUCT)))
PLATFORM := isharkl2
else
PLATFORM := default
endif
$(warning TOP:$(TOP) LOCAL_PATH:$(LOCAL_PATH) TARGET_PRODUCT:$(TARGET_PRODUCT)  platform:$(PLATFORM))
LOCAL_C_INCLUDES:= \
    $(TOP)/vendor/sprd/proprietories-source/engmode \
    $(TOP)/vendor/sprd/proprietories-source/engmode/gpio \
    $(LOCAL_PATH)/$(PLATFORM)

LOCAL_SHARED_LIBRARIES:= \
    libcutils \
    liblog \
    libc

include $(BUILD_SHARED_LIBRARY)
