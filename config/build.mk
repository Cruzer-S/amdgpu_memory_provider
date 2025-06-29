CC := hipcc
CXX := hipcc

OUTPUT := hip_devmem_tcp

CPPFLAGS += -D__HIP_PLATFORM_AMD__

LDLIBS += -lhsa-runtime64 -lamdhip64
