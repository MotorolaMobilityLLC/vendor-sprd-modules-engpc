
file_list += $(wildcard $(LOCAL_PATH)/android/*.cpp)
LOCAL_C_INCLUDES    += $(LOCAL_PATH)/android/

ifneq ($(filter $(strip $(PLATFORM_VERSION)),Q 10 R 11),)
    LOCAL_CFLAGS += -DENGPC_VSER_INIT
endif

