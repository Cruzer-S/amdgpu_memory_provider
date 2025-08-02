CC := hipcc
CXX := hipcc

CPPFLAGS += -D__HIP_PLATFORM_AMD__

LDLIBS += -lamdhip64
