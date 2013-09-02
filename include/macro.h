#ifndef MACRO_H
#define MACRO_H

#define __SWAP16(x) (((x<<8)|((x>>8)&0xff))&0xffff)
#define __SWAP32(x) ((x<<24)|((x<<8)&0xff0000)|((x>>8)&0xff00)|((x>>24)&0xff))


#endif // MACRO_H
