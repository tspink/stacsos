this-dir := $(CURDIR)
target := $(out-dir)/stacsos.a

src-dir := $(this-dir)/src
inc-dir := $(this-dir)/inc

c-srcs := $(shell find $(src-dir) | grep -E "\.cpp$$")
a-srcs := $(shell find $(src-dir) | grep -E "\.S$$")
objs := $(c-srcs:.cpp=.o) $(a-srcs:.S=.o)
deps := $(c-srcs:.cpp=.d)

cxxflags := -I $(inc-dir)
cxxflags += -nostdinc -nostdlib -include $(inc-dir)/global.h
cxxflags += -std=gnu++23 -O3 -Wall -g
cxxflags += -march=native
cxxflags += -ffreestanding -fno-builtin -fno-omit-frame-pointer -fno-rtti -fno-exceptions
cxxflags += -fno-delete-null-pointer-checks -fcheck-new -mno-red-zone -fno-stack-protector
cxxflags += -mno-mmx -mno-sse -mno-sse2 -mno-sse3 -mno-ssse3 -mno-sse4.1 -mno-sse4.2
cxxflags += -mno-sse4 -mno-avx -mno-aes -mno-sse4a -mno-fma4 -fPIE

build: $(target)

clean: .FORCE
	rm -f $(objs) $(deps) $(target)

$(target): $(objs)
	@echo "  AR    $@"
	$(q)ar rcs $@ $(objs)

%.o: %.cpp %.d
	@echo "  C++   $@"
	$(q)g++ -c -o $@ $(cxxflags) $<

%.d: %.cpp
	$(q)g++ -M -MT $(@:.d=.o) -o $@ $(cxxflags) $<

%.o: %.S
	@echo "  AS    $@"
	$(q)g++ -c -o $@ $(asflags) $<

.PHONY: .FORCE

-include $(deps)
