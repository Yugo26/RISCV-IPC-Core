#define INFO(f_, ...) kprintf((f_), ##__VA_ARGS__)

#ifndef DEBUGMSG
    #define DEBUG(f_, ...)
    #define DEBUG_ISR(f_, ...)
#else
    #define DEBUG(f_, ...) do{\
            if (getCurrentTask()==NULL) \
                kprintf("kernel, %s::",__FUNCTION__);\
            else\
                kprintf("%s, %s::", getCurrentTask()->name,__FUNCTION__);\
            kprintf((f_), ##__VA_ARGS__);\
        }while(0)
    #define DEBUG_ISR(f_, ...) do{\
            if (getCurrentTask()==NULL) \
                lockfree_printf("kernel, %s::",__FUNCTION__);\
            else\
                lockfree_printf("%s, %s::", getCurrentTask()->name,__FUNCTION__);\
            lockfree_printf((f_), ##__VA_ARGS__);\
        }while(0)
#endif
