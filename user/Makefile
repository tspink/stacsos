this-dir := $(CURDIR)

apps := init shell sched-test mandelbrot cat poweroff sched-test2

app-dirs := $(foreach APP,$(apps),$(this-dir)/$(APP))
export app-target-dir := $(out-dir)/rootfs/usr

build-targets := $(patsubst %,__build__%,$(app-dirs))
BUILD-TARGET = $(patsubst __build__%,%,$@)

clean-targets := $(patsubst %,__clean__%,$(app-dirs))
CLEAN-TARGET = $(patsubst __clean__%,%,$@)

fs-target := $(out-dir)/rootfs.img.tar

build: $(build-targets) $(fs-target)
	@

clean: $(clean-targets)
	@

$(build-targets): $(app-target-dir) __build_ulib
	@make -C $(BUILD-TARGET) -f ../Makefile.user target=$(notdir $(BUILD-TARGET)) build

$(clean-targets): __clean_ulib
	@make -C $(CLEAN-TARGET) -f ../Makefile.user target=$(notdir $(CLEAN-TARGET)) clean

__build_ulib:
	@make -C $(this-dir)/ulib build

__clean_ulib:
	@make -C $(this-dir)/ulib clean

$(fs-target): .FORCE
	@echo "  CP sysroot"
	$(q)cp -r $(top-dir)/sysroot/* $(out-dir)/rootfs/
	@echo "  TAR   $(fs-target)"
	$(q)tar cf $(fs-target) -C $(out-dir)/rootfs .

$(app-target-dir):
	@mkdir -p $(app-target-dir)

.PHONY: .FORCE
