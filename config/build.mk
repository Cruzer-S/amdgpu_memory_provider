CC := hipcc
CXX := hipcc

CPPFLAGS += -D__HIP_PLATFORM_AMD__
CFLAGS := -g

LDLIBS := -lhsa-runtime64
