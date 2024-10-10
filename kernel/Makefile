this-dir := $(CURDIR)
target := $(out-dir)/stacsos

src-dir := $(this-dir)/src
inc-dir := $(this-dir)/inc

linker-script := $(this-dir)/stacsos.ld

c-srcs := $(shell find $(src-dir) | grep -E "\.cpp$$")
a-srcs := $(shell find $(src-dir) | grep -E "\.S$$")
objs := $(c-srcs:.cpp=.o) $(a-srcs:.S=.o)
deps := $(c-srcs:.cpp=.d)

cxxflags := -I $(lib-inc-dir) -I $(inc-dir) -include $(inc-dir)/stacsos/kernel/kernel-global.h
cxxflags += -mcmodel=kernel
cxxflags += -nostdinc -nostdlib -include $(lib-inc-dir)/global.h
cxxflags += -std=gnu++23 -O3 -Wall -g
cxxflags += -march=native -no-pie -fno-pic
cxxflags += -ffreestanding -fno-builtin -fno-omit-frame-pointer -fno-rtti -fno-exceptions
cxxflags += -fno-delete-null-pointer-checks -fcheck-new -mno-red-zone -fno-stack-protector
cxxflags += -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2
cxxflags += -mno-sse4 -mno-avx -mno-aes -mno-sse4a -mno-fma4

asflags := -nostdinc -nostdlib -Wall -g -ffreestanding -fno-builtin
ldflags := -nostdlib -z nodefaultlib -no-pie

fonts := zap-light16.psf zap-vga16.psf tamsyn-8x15r.psf
font-objects := $(patsubst %.psf,%.o,$(fonts))

build: $(target)

clean: .FORCE
	rm -rf $(objs) $(deps) $(target) $(font-objects)

$(target).64: $(linker-script) $(objs) $(lib) $(font-objects)
	@echo "  LD    $@"
	$(q)g++ -o $@ $(ldflags) -T $(linker-script) $(objs) $(font-objects) $(lib)

$(target): $(target).64
	@echo "  OBJCOPY $@"
	$(q)objcopy --input-target=elf64-x86-64 --output-target=elf32-i386 $(target).64 $@

%.o: %.psf
	@echo "  OBJCOPY $@"
	$(q)objcopy -O elf64-x86-64 -B i386 -I binary $< $@

%.o: %.cpp %.d
	@echo "  CXX   $@"
	$(q)g++ -c -o $@ $(cxxflags) $<

%.d: %.cpp
	$(q)g++ -M -MT $(@:.d=.o) -o $@ $(cxxflags) $<

%.o: %.S
	@echo "  AS    $@"
	$(q)g++ -c -o $@ $(asflags) $<

.PHONY: .FORCE

-include $(deps)
