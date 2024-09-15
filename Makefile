export MAKEFLAGS    += -rR --no-print-directory
export q            := @
export arch         ?= x86

export top-dir := $(CURDIR)
export build-dir := $(top-dir)/build
export out-dir := $(top-dir)/out
export lib := $(out-dir)/stacsos.a
export lib-inc-dir := $(top-dir)/lib/inc

targets := lib kernel user

build-targets := $(patsubst %,__build__%,$(targets))
BUILD-TARGET = $(patsubst __build__%,%,$@)

clean-targets := $(patsubst %,__clean__%,$(targets))
CLEAN-TARGET = $(patsubst __clean__%,%,$@)

qemu ?= qemu-system-x86_64
qemu-display ?= gtk

all: $(build-targets)
clean: $(clean-targets)

run: all
	$(qemu) \
		-smp 4 \
		-machine q35 \
		-display $(qemu-display) \
		-enable-kvm \
		-m 8G \
		-debugcon stdio \
		-cpu host \
		-kernel $(out-dir)/stacsos \
		-hda $(out-dir)/rootfs.img.tar

debug: all
	$(qemu) \
		-s -S \
		-smp 4 \
		-machine q35 \
		-display $(qemu-display) \
		-m 8G \
		-debugcon stdio \
		-kernel $(out-dir)/stacsos

__build__%: .FORCE
	@make -C $(top-dir)/$(BUILD-TARGET) build

__clean__%: .FORCE
	@make -C $(top-dir)/$(CLEAN-TARGET) clean

__build__kernel __build__user: $(lib)

$(lib): __build__lib

.PHONY: .FORCE
