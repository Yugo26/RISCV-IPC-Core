#include "os.h"

void*(*sysFunc[MAX_SYSCALL])(void *p);


void syscall_register(uint16_t num, void *(*sysfunc)(void *p)){
    reg_t lock_status = baseLock();
    sysFunc[num] = sysfunc;
    baseUnLock(lock_status);
}

void do_syscall(ctx_t *ctx)
{
	uint32_t syscall_num = ctx->a7;
	DEBUG_ISR("syscall_num: %d\n", syscall_num);

    if (sysFunc[syscall_num])
        ctx->a0 = sysFunc[syscall_num]((void*)ctx->a0);
    else{
        DEBUG_ISR("Unknown syscall no: %d\n", syscall_num);
		ctx->a0 = 0;   
    }
	return;
}