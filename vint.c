/**
    Copyright (C) powturbo 2013-2016
    GPL v2 License
  
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

    - homepage : https://sites.google.com/site/powturbo/
    - github   : https://github.com/powturbo
    - twitter  : https://twitter.com/powturbo
    - email    : powturbo [_AT_] gmail [_DOT_] com
**/
//   vint.c - "Integer Compression" variable byte     
  #ifndef USIZE
#include <stdio.h>
#include <string.h>

#include "conf.h"
#include "vint.h"
#include "bitutil.h"

#define UN 8 // 4 //

#define VDELTA    0
#define VBDENC    vbdenc
#define VBDDEC    vbddec
#define VBDGETX   vbdgetx
#define VBDGETGEQ vbdgetgeq

#define USIZE 32
#include __FILE__
#undef USIZE
 
#define USIZE 64
#include __FILE__
#undef USIZE

#define USIZE 16
#include __FILE__
#undef USIZE

#define VDELTA 1
#define VBDENC    vbd1enc
#define VBDDEC    vbd1dec
#define VBDGETX   vbd1getx
#define VBDGETGEQ vbd1getgeq

#define USIZE 32
#include __FILE__
#undef USIZE
 
#define USIZE 64
#include __FILE__
#undef USIZE

#define USIZE 16
#include __FILE__
#undef USIZE

  #else
#define uint_t TEMPLATE3(uint, USIZE, _t)

    #if VDELTA == 0
#define OVERFLOWD(in,n,out,vbmax)    if(*in == vbmax) { memcpy(out, in+1, n*(USIZE/8)); return in+1+n*(USIZE/8); }
#define OVERFLOWE(in,n,out,op,vbmax) if(op > out + n*(USIZE/8)) { *out = vbmax; memcpy(out+1, in, n*(USIZE/8)); op = out+1+n*(USIZE/8); }

//#define RLE(_ip_,_op_) if(_ip_+1 < e && *_ip_ == *(_ip_+1)) { uint_t *_q = _ip_+1; while(_q+1 < e && *(_q+1) == *_ip_) _q++; unsigned _r = _q - _ip_;\
//  { _ip_+=_r; _r--; *_op_++= _ip_[0]?252:253; vbput32(op, _r); if(_ip_[0]) TEMPLATE2(vbput, USIZE)(op, _ip_[0]);  }}
//#define RLE(_ip_,_op_)

unsigned char *TEMPLATE2(vbdec, USIZE)(unsigned char  *__restrict in, unsigned n, uint_t *__restrict out) { 
  register uint_t x, *op; 
  OVERFLOWD(in, n, out, VB_MAX);
  #define VBE(_i_) TEMPLATE2(_vbget, USIZE)(in, x, op[_i_] = x)
  for(op = out; op != out+(n&~(UN-1)); op += UN) { VBE(0); VBE(1); VBE(2); VBE(3); __builtin_prefetch(in+8*USIZE, 0);
	  #if UN > 4
    VBE(4); VBE(5); VBE(6); VBE(7);	  
      #endif
  }
  while(op != out+n) TEMPLATE2(_vbget, USIZE)(in, x, *op++ = x );
  return in;
}
#undef VBE

unsigned char *TEMPLATE2(vbenc, USIZE)(uint_t *__restrict in, unsigned n, unsigned char *__restrict out) {
  register uint_t x, *ip, *e=in+n; 
  unsigned char *op = out;
  #define VBD(_i_) x = ip[_i_]; TEMPLATE2(_vbput, USIZE)(op, x, ;);
  for(ip = in; ip != in+(n&~(UN-1)); ip += UN) {  		__builtin_prefetch(ip+USIZE*8, 0);
    VBD(0); VBD(1); VBD(2); VBD(3); 
	  #if UN > 4
    VBD(4); VBD(5); VBD(6); VBD(7);
	  #endif
  }
  while(ip != in+n) { 
    x = *ip++; 
	TEMPLATE2(_vbput, USIZE)(op, x, ;); 
  }
  OVERFLOWE(in,n,out,op,VB_MAX);
  return op;
}
#undef VBD

uint_t TEMPLATE2(vbgetx, USIZE)(unsigned char  *__restrict in, unsigned idx) { 
  unsigned char *ip;
  unsigned i;
  uint_t x;
  if(*in == 255) return TEMPLATE2(ctou, USIZE)(in+1+idx*(USIZE/8));
  for(ip = in,i = 0; i <= idx; i++)
	ip += TEMPLATE2(_vbvlen, USIZE)(*ip);
  TEMPLATE2(_vbget, USIZE)(in, x, ;);
  return x;
}

/*unsigned TEMPLATE2(vbgeteq, USIZE)(unsigned char *__restrict in, unsigned n, uint_t key, unsigned char **__restrict _ip) { 
  unsigned i;
  unsigned char *ip;
  uint_t x;
  if(*in == 255) { 
    for(ip = (*_ip==in)?in:*ip; ip < in+n; ip+USIZE/8) {
	  TEMPLATE2(_vbget, USIZE)(ip, x, ;);
	  if((x = TEMPLATE2(ctou, USIZE)(ip)) == key) break;
    }  
  } else for(ip = *_ip,i=idx; i < n; i++) {
	TEMPLATE2(_vbget, USIZE)(ip, x, ;);
	if(x == key) break;
  }
  *_ip = ip;
  return i;
}*/
unsigned char *TEMPLATE2(vbzenc, USIZE)(uint_t *__restrict in, unsigned n, unsigned char *__restrict out, uint_t start) { 
  uint_t *ip,v;
  unsigned char *op = out;
  #define VBZE { v = TEMPLATE2(zigzagenc, USIZE)((*ip)-start); start=*ip++; TEMPLATE2(_vbput, USIZE)(op, v, ;); }
  for(ip = in; ip != in+(n&~(4-1)); ) { VBZE;VBZE;VBZE;VBZE; }
  while(ip <  in+n) VBZE; 
  //OVERFLOWE(in,n,out,op);
  return op;
}  
#undef VBZE

unsigned char *TEMPLATE2(vbzdec, USIZE)(unsigned char *__restrict in, unsigned n, uint_t *__restrict out, uint_t start) { 
  uint_t x,*op;
  #define VBZD { TEMPLATE2(_vbget, USIZE)(in, x, ;); *op++ = (start += TEMPLATE2(zigzagdec, USIZE)(x)); }
  for(op = out; op != out+(n&~(UN-1)); ) { VBZD; VBZD; VBZD; VBZD;
	  #if UN > 4
    VBZD; VBZD; VBZD; VBZD;
      #endif
  }
  while(op != out+n) VBZD; 
  return in;
}
#undef VBZD

uint_t TEMPLATE2(vbzgetx, USIZE)(unsigned char  *__restrict in, unsigned idx, uint_t start) {
  unsigned char *ip;
  unsigned i;
  uint_t x;
  for(ip = in,i = 0; i <= idx; i++) {
	TEMPLATE2(_vbget, USIZE)(ip, x, ;);
    start += x+1;
  }
  return start;
}

unsigned TEMPLATE2(vbzgeteq, USIZE)(unsigned char **__restrict in, unsigned n, unsigned idx, uint_t key, uint_t start ) { 
  unsigned i;
  unsigned char *ip;
  uint_t x;
  for(ip = *in,i=idx; i < n; i++) {
	TEMPLATE2(_vbget, USIZE)(ip, x, ;);
	if((start += x+1) == key) 
      break;
  }
  *in = ip;
  return i;
}
  #endif

unsigned char *TEMPLATE2(VBDENC, USIZE)(uint_t *__restrict in, unsigned n, unsigned char *__restrict out, uint_t start) {
  unsigned char *op = out; if(!n) return out;
  uint_t *ip, b=0,v /*,x=in[0]-start-VDELTA*/; 
  #define VBDE { v = (*ip)-start-VDELTA; start = *ip++; TEMPLATE2(_vbput, USIZE)(op, v, ;); b |= (v /*^ x*/); }
  for(ip = in; ip != in + (n&~(UN-1)); ) { VBDE;VBDE;VBDE;VBDE; 
	  #if UN > 4
    VBDE; VBDE; VBDE; VBDE; 	  
      #endif
  }
  while(ip != in+n) VBDE;
  if(!b) { op = out; *op++ = VB_MAX; } // if (x) { op = out; *op++ = VB_MAX-2; TEMPLATE2(_vbput, USIZE)(op, x, ;); }
    #if USIZE < 64
  OVERFLOWE(in,n,out,op,VB_MAX-1);
    #endif
  return op;
}
#undef VBDE

unsigned char *TEMPLATE2(VBDDEC, USIZE)(unsigned char *__restrict in, unsigned n, uint_t *__restrict out, uint_t start) { 
  uint_t x,*op; 
  if(!n) return in;
    #if USIZE < 64
  OVERFLOWD(in,n,out,VB_MAX-1);
    #endif
  
  if(in[0] == VB_MAX) { 
    in++; 
      #if defined(__SSE2__) && USIZE == 32
	    #if VDELTA == 0
	if(n) BITZERO32(out, n, start);
	    #else
	if(n) BITDIZERO32(out, n, start, VDELTA);
        #endif
	  #else 
	    #if VDELTA == 0
    for(x = 0; x < n; x++) out[x] = start;
	    #else
    for(x = 0; x < n; x++) out[x] = start+x+VDELTA;
        #endif		
      #endif
	return in;
  }
    #if 0 //USIZE < 64
  else if(in[0] == VB_MAX-2) { in++; 
    uint_t z; 
    TEMPLATE2(_vbget, USIZE)(in, z, ;);
	  #if VDELTA == 0
    for(x = 0; x < n; x++) out[x] = start+z;
      #else
    for(x = 0; x < n; x++) out[x] = start+x+z;
      #endif
	return in;
  }
    #endif
  #define VBDD { TEMPLATE2(_vbget, USIZE)(in, x, x+=VDELTA); *op++ = (start += x); }
  for(op = out; op != out+(n&~(UN-1)); ) { 
    VBDD; VBDD; VBDD; VBDD; 
	  #if UN > 4
    VBDD; VBDD; VBDD; VBDD; 	  
      #endif
  }
  while(op != out+n) VBDD;
  return in;
}
#undef VBDD

uint_t TEMPLATE2(VBDGETX, USIZE)(unsigned char  *__restrict in, unsigned idx, uint_t start) {
  unsigned char *ip;
  unsigned i;
  uint_t x;

    #if USIZE > 64
  unsigned long long u; 
  _vbget64(in, u, ;); x = u>>1; start += x+1;
  if(u & 1) return start + ; 
    #endif
  for(ip = in; i <= idx; i++) {
	TEMPLATE2(_vbget, USIZE)(ip, x, ;);
    start += x+1;
  }
  return start;
}

unsigned TEMPLATE2(VBDGETGEQ, USIZE)(unsigned char **__restrict in, unsigned n, unsigned idx, uint_t *key, uint_t start ) { 
  unsigned i=0;
  unsigned char *ip;
  uint_t x;
    #if USIZE < 64
  if(!idx) {
    unsigned long long u; _vbget64(in, u, ;); x = u>>1; start += x+1;
    if((u & 1) && start == *key) { *in = ip; return 0; }
    i++; 
  }
    #endif
  for(ip = *in; i < n; i++) {
	TEMPLATE2(_vbget, USIZE)(ip, x, ;);
	if((start += x+VDELTA) == *key) 
      break;
  }
  *in = ip;
  return i;
}
#undef uint_t 
#endif
