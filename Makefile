KDIR  := /lib/modules/$(shell uname -r)/build
BDIR  := $(PWD)/build

all: $(BDIR)
	cp -u ftrace_clock_gettime.c $(BDIR)/
	cp -u Kbuild $(BDIR)/
	$(MAKE) -C $(KDIR) M=$(BDIR) modules

clean:
	$(MAKE) -C $(KDIR) M=$(BDIR) clean 2>/dev/null; true
	rm -rf $(BDIR)

$(BDIR):
	mkdir -p $(BDIR)
