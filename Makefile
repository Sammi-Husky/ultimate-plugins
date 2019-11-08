#---------------------------------------------------------------------------------
.SUFFIXES:
#---------------------------------------------------------------------------------

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>/devkitpro")
endif

TOPDIR ?= $(CURDIR)
include $(DEVKITPRO)/devkitA64/base_rules

all: plugins/ledgehogging.elf plugins/lua_hash_intercept.elf

clean:
	@rm -rf plugins
	@cd ledgehogging && make clean
	@cd lua_hash_intercept && make clean

ledgehogging/ledgehogging.elf: 
	@cd ledgehogging && make

lua_hash_intercept/lua_hash_intercept.elf:
	@cd lua_hash_intercept && make

# outputs
plugins/ledgehogging.elf: ledgehogging/ledgehogging.elf
	@mkdir -p plugins
	@cp $< $@

plugins/lua_hash_intercept.elf: lua_hash_intercept/lua_hash_intercept.elf
	@mkdir -p plugins
	@cp $< $@