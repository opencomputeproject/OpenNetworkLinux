include $(ONL)/make/any.mk


liboom_shim.so:
	@$(CC) -o liboom_shim.so -shared -PIC liboom_shim.c

.PHONY: deb
deb:
	$(MAKE) -C deb
