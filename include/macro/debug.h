#ifndef __DEBUG_H
#define __DEBUG_H

#define DBG_LEVEL_MIN  	0
#define DBG_LEVEL_MAX  	255
#define DBG(l,x)  if(l>=DBG_LEVEL_MIN && l<=DBG_LEVEL_MAX) x;

#endif