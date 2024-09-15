this-dir := $(CURDIR)
target := $(out-dir)/ulib.a

src-dir := $(this-dir)/src
inc-dir := $(this-dir)/inc
lib-inc-dir := $(top-dir)/lib/inc

c-srcs := $(shell find $(src-dir) | grep -E "\.cpp$$")
a-srcs := $(shell find $(src-dir) | grep -E "\.S$$")
objs := $(c-srcs:.cpp=.o) $(a-srcs:.S=.o)
deps := $(c-srcs:.cpp=.d)

cxxflags := -I $(inc-dir) -I $(lib-inc-dir)
cxxflags += -nostdinc -nostdlib -include $(lib-inc-dir)/global.h
cxxflags += -std=gnu++23 -O3 -Wall -g
cxxflags += -march=native -mno-sse
cxxflags += -ffreestanding -fno-builtin -fno-omit-frame-pointer -fno-rtti -fno-exceptions -fno-stack-protector
cxxflags += -fno-delete-null-pointer-checks -fcheck-new -fPIE

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
