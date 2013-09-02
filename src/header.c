#include <libdragon.h>
#include <header.h>
#include <string.h>

void read_header(uint8_t *buf, header *hdr) {
	memcpy(hdr, buf,sizeof(header));
}
