#include <cd64.h>
#include <libdragon.h>

void cd64_init() {
	cd64_enable();
	io_write(CD64_REG_MODE,CD64_MODE_0);
}

void cd64_enable() { 
	io_write(CD64_REG_MODE_ENABLE,CD64_ENABLED);
}
void cd64_disable() {
	io_write(CD64_REG_MODE_ENABLE,CD64_DISABLED);
}
