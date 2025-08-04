CC := hipcc
CXX := hipcc

CPPFLAGS += -D__HIP_PLATFORM_AMD__

LDFLAGS := -lhsa-runtime64
