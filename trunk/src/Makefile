ROOT_DIR:=..

include $(ROOT_DIR)/Makefile.include

MODULES:=core pbr pipeline

all:
	@for i in $(MODULES); do \
		$(MAKE) -C $$i; \
		done;


clean:
	@for i in $(MODULES); do \
		$(MAKE) clean -C $$i; \
		done;


