# Makefile for Early Loop Detector Plugin

PLUGIN_NAME = early_loop_detector
PLUGIN_SOURCE = src/early_loop_detector.cc
PLUGIN_OBJECT = $(PLUGIN_NAME).so

# Get GCC plugin directory
GCC_PLUGIN_DIR := /home/avinashd/cross/gcc-powerpc64le/install_x86/lib/gcc/x86_64-linux-gnu/16.0.1/plugin

# Compiler flags
CXX = g++
CXXFLAGS = -I$(GCC_PLUGIN_DIR)/include -fPIC -fno-rtti -O2 -Wall -g
LDFLAGS = -shared

all: $(PLUGIN_OBJECT)

$(PLUGIN_OBJECT): $(PLUGIN_SOURCE)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $< -o $@

clean:
	rm -f $(PLUGIN_OBJECT)

test: $(PLUGIN_OBJECT)
	gcc -fplugin=./$(PLUGIN_OBJECT) -c test.c

test-verbose: $(PLUGIN_OBJECT)
	gcc -fplugin=./$(PLUGIN_OBJECT) \
	     -fplugin-arg-$(PLUGIN_NAME)-verbose \
	     -c test.c

.PHONY: all clean test test-verbose

