#include "types.h"

void *memset(void *ptr, int value, uint32_t num) {
    unsigned char *p = (unsigned char *) ptr;
    while (num-- > 0) {
        *p++ = (unsigned char) value;
    }
    return ptr;
}

char* memcpy(void* dest,const void* src, uint32_t num) 
{ 
   char* p_dest = (char*)dest; 
   const char* p_src = (const char*)src;

   while(num-- > 0)
     *p_dest++ = *p_src++;
   return dest; 
}

int strcmp(const char *s1, const char *s2)
{
	while (*s1 == *s2++)
		if (*s1++ == '\0')
			return (0);
	return (*(const unsigned char *)s1 - *(const unsigned char *)(s2 - 1));
}

