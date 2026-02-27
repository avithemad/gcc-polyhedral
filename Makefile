# Makefile for Early Loop Detector Plugin

PLUGIN_NAME = early_loop_detector
PLUGIN_SOURCE = src/early_loop_detector.cc
PLUGIN_OBJECT = $(PLUGIN_NAME).so

# Get GCC plugin directory
GCC_PLUGIN_DIR := $(shell g++ -print-file-name=plugin)

# Compiler flags
CXX = g++
CXXFLAGS = -I$(GCC_PLUGIN_DIR)/include -fPIC -fno-rtti -O0 -Wall -g
LDFLAGS = -shared

all: $(PLUGIN_OBJECT)

$(PLUGIN_OBJECT): $(PLUGIN_SOURCE)
	@echo CXX: $(CXX)
	@echo CXXFLAGS: $(CXX)
	@echo GCC_PLUGIN_DIR: $(GCC_PLUGIN_DIR)
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

