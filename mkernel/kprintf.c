/*
  _           _ 
   | . |   | |_|   Kernel printf function
  \  | | | | |  
 
*/

#include <kernel.h>
#include <console.h>
#include <asm/386io.h>
#include <stdarg.h>

char* ultoa_end(unsigned long int v,char* d,int radix){
  const  char c[] = "0123456789abcdef";
  char b[33];
  char* p = b + sizeof b;

  *--p = 0;
  do {
	*--p = c[v%radix];
	v /= radix;
  } while (v);
  while((*d=*p)){
	++d;
	++p;
  }
  return d;
}

static void dummy_print_char(int c){
  kputchar(c);
}

static void	(*print_char)(int) = dummy_print_char;

static void one_char(char c){
//	if(c == '\n') print_char('\r');
  print_char(c);
}

static void vmsg(const char *fmt,va_list args){
  char*         vs;
  unsigned long	num;
  unsigned		radix;
  int			dig;
  char			buf[33], cc;
	
  for(;*fmt;++fmt){
	if(*fmt == '%'){
	  radix = dig = 0;
	  
	  switch(*++fmt){
		
	  case 'b':
		num = va_arg(args,unsigned);
		dig = 2;
		radix = 16;
		break;
		
	  case 'w':
		num = va_arg(args,unsigned);
		dig = 4;
		radix = 16;
		break;
				
	  case 'x':
	  case 'X':
		num = va_arg(args,unsigned);
		dig = 4;
		radix = 16;
		break;
		
	  case 'l':
		num = va_arg(args,unsigned long);
		dig = 8;
		radix = 16;
		break;
		
	  case 'd':
		num = va_arg(args,unsigned);
		radix = 10;
		break;
		
	  case 's':
		vs = va_arg(args,char*);
		while (*vs) one_char(*vs++);
		continue;
				
	  default:
		one_char(*fmt);
		continue;
	  }
	  
	  dig -= ultoa_end(num,buf,radix)-buf;
			
	  while(dig>0){
		print_char('0');
		--dig;
	  }
	  
	  for(vs=buf;*vs!='\0';++vs) print_char(*vs);
	} else {
	  one_char(*fmt);
	}
  }
}
				
void kprintf(const char* fmt,...){
  va_list args;
  va_start(args,fmt);
  vmsg(fmt,args);
  va_end(args);
}	

