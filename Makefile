# Makefile for SvaklaAI project

# Intel oneAPI paths
ONEAPI_ROOT = /opt/intel/oneapi
ONEAPI_COMPILER = $(ONEAPI_ROOT)/compiler/latest
ONEAPI_MKL = $(ONEAPI_ROOT)/mkl/latest

# Check for required oneAPI components
ONEAPI_EXISTS := $(shell test -d $(ONEAPI_ROOT) && echo 1 || echo 0)
ONEAPI_DPL_EXISTS := $(shell test -f $(ONEAPI_COMPILER)/include/oneapi/dpl/algorithm && echo 1 || echo 0)
ONEAPI_SYCL_EXISTS := $(shell test -f $(ONEAPI_COMPILER)/include/sycl/sycl.hpp && echo 1 || echo 0)

# Compiler and base flags
CXX = g++
CXXFLAGS = -std=c++23 -Wall -Wextra -march=native -O3 -pthread

# System includes
CXXFLAGS += -I/usr/include/nlohmann
CXXFLAGS += $(shell pkg-config --cflags dbus-1)
CXXFLAGS += -I/usr/include/openssl
CXXFLAGS += -I/usr/include/zlib-ng
CXXFLAGS += -I.

# oneAPI configuration
ifeq ($(ONEAPI_EXISTS),1)
    CXXFLAGS += -I$(ONEAPI_COMPILER)/include
    ifeq ($(ONEAPI_SYCL_EXISTS),1)
        CXXFLAGS += -I$(ONEAPI_COMPILER)/include/sycl
        LDFLAGS += -L$(ONEAPI_COMPILER)/lib -Wl,-rpath,$(ONEAPI_COMPILER)/lib
        LDFLAGS += -lsycl -lOpenCL
    endif
    ifeq ($(ONEAPI_DPL_EXISTS),1)
        CXXFLAGS += -DUSE_ONEAPI_DPL
        LDFLAGS += -L$(ONEAPI_MKL)/lib/intel64 -Wl,-rpath,$(ONEAPI_MKL)/lib/intel64
        LDFLAGS += -lmkl_sycl -lmkl_intel_ilp64 -lmkl_sequential -lmkl_core
    else
        CXXFLAGS += -DUSE_STD_PARALLEL
    endif
endif

# Add dependency tracking
DEPFLAGS = -MMD -MP
CXXFLAGS += $(DEPFLAGS)

# Add subdirectory includes
CXXFLAGS += $(addprefix -I,$(sort $(dir $(SRCS))))

# Linker flags
LDFLAGS += -lboost_system -lboost_filesystem -lboost_thread
LDFLAGS += -lssl -lcrypto -lcurl -lpthread -llmdb -ldl
LDFLAGS += -lboost_program_options
LDFLAGS += $(shell pkg-config --libs dbus-1)
LDFLAGS += -lz-ng

# Source files
SRCS  = $(wildcard *.cpp)
SRCS += $(wildcard api/*.cpp)
SRCS += $(wildcard chat/*.cpp)
SRCS += $(wildcard ethics/*.cpp)
SRCS += $(wildcard logic/*.cpp)
SRCS += $(wildcard model/*.cpp)
SRCS += $(wildcard nlp/*.cpp)
SRCS += $(wildcard plugins/*.cpp)
SRCS += $(wildcard programming/*.cpp)
SRCS += $(wildcard scraping/*.cpp)
SRCS += $(wildcard web_interface/*.cpp)

# Object files
OBJS = $(SRCS:.cpp=.o)

# Target executable
TARGET = svaklaai

# Default directories (user-specific)
PREFIX      ?= $(HOME)/svakla
PLUGIN_DIR  ?= $(PREFIX)/plugins
CONFIG_DIR  ?= $(PREFIX)/config
DATA_DIR    ?= $(PREFIX)/data
CHAT_DIR    ?= $(PREFIX)/chat

# Build rules
all: check-oneapi $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Include generated dependency files
-include $(OBJS:.o=.d)

install: $(TARGET)
	install -d $(PREFIX)/bin
	install -d $(PLUGIN_DIR)
	install -d $(CONFIG_DIR)
	install -d $(DATA_DIR)
	install -d $(CHAT_DIR)
	install $(TARGET) $(PREFIX)/bin/
	chmod 755 $(PREFIX)/bin/$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

# Add rule to check oneAPI setup
check-oneapi:
	@if [ ! -d "$(ONEAPI_ROOT)" ]; then \
		echo "Error: oneAPI not found in $(ONEAPI_ROOT)"; \
		echo "Please install oneAPI or update ONEAPI_ROOT path"; \
		exit 1; \
	fi
	@echo "oneAPI found in $(ONEAPI_ROOT)"
	@if [ "$(ONEAPI_DPL_EXISTS)" = "1" ]; then \
		echo "oneAPI DPL headers found"; \
	else \
		echo "Warning: oneAPI DPL headers not found, using STD parallel algorithms"; \
	fi

.PHONY: all install clean check-oneapi
