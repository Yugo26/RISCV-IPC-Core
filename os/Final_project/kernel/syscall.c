#include "os.h"

extern void*(*sysFunc[])(void *p);

//ISR do_syscall here
int sys_gethid(unsigned int *ptr_hid)
{
	DEBUG_ISR("--> sys_gethid, arg0 = 0x%x\n", ptr_hid);
	if (ptr_hid == NULL) {
		DEBUG_ISR("ptr_hid == NULL\n");
       		return -1;
	} else {
		DEBUG_ISR("ptr_hid != NULL\n");
		 *ptr_hid = r_mhartid();
 		 return 0;
	}
}

static void * _sys_gethid(void *p) {
    int rst = sys_gethid((unsigned int *)p);
    return (void *)rst;
}

static void *_sys_delay(void *p) {
    taskDelay((uint32_t)p);
    return NULL;
}

static void * _sem_create(void *p) {
    uint32_t *v = (uint32_t*)p;
    err_t i = createSem((uint16_t)v[0],(uint16_t)v[1],(uint8_t)v[2]);
    return (void*)i;
}

static void * _sem_take(void *p) {
    int *v=(int*)p;
    err_t i = sem_take((uint16_t)v[0], v[1]);
    return (void*)i;
}

static void * _sem_release(void *p) {
    err_t i = sem_release((uint16_t)p);
    return (void*)i;
}

static void * _mut_create(void *p){
    uint32_t *v = (uint32_t*)p;
    err_t i = createMut((uint16_t)v[0],(uint16_t)v[1],(uint8_t)v[2]);
    return (void*)i;
}

static void * _mut_take(void *p) {
    int *v=(int*)p;
    err_t i = mut_take((uint16_t)v[0], v[1]);
    return (void*)i;
}

static void * _mut_release(void *p) {
    err_t i = mut_release((uint16_t)p);
    return (void*)i;
}

static void * _msgq_create(void *p) {
    uint16_t *v = (uint16_t *)p; 
    err_t i = MsgQCreate((uint16_t)v[0], (uint16_t)v[1]);
    return (void*)i;
}

static void * _msgq_send(void *p) {
    int *v = (int*)p;
    err_t i = MsgQSend((uint16_t)v[0], (void*)v[1], (uint32_t)v[2], v[3]);
    return (void*)i;
}

static void * _msgq_recv(void *p) {
    int *v = (int*)p; 
    err_t i = MsgQRecv((uint16_t)v[0], (void*)v[1], (uint32_t)v[2], v[3]);
    return (void*)i;
}

static void * _event_create(void *p){
    err_t i = eventCreate();
    return(void*)i;
}

static void * _event_wait(void *p){
    int *v = (int *)p;
    err_t i = event_wait((uint32_t)v[0], (uint32_t)v[1], (uint8_t)v[2], (uint32_t)v[3]);
    return (void*)i;
}

static void *_event_set(void *p){
    int *v = (int*)p;
    err_t i = event_set((uint32_t)v[0], (uint32_t)v[1]);
    return (void*)i;
}

void syscall_init() {
    for (int i=0; i<MAX_SYSCALL;i++)
        sysFunc[i]=NULL;
    
    //register all syscall here
    syscall_register(GETHID, _sys_gethid);
    syscall_register(DELAY, _sys_delay);
    syscall_register(SEM_TAKE, _sem_take);
    syscall_register(SEM_RELEASE, _sem_release);
    syscall_register(SEM_CREATE, _sem_create);
    syscall_register(MUT_TAKE, _mut_take);
    syscall_register(MUT_RELEASE, _mut_release);
    syscall_register(MUT_CREATE, _mut_create);
    syscall_register(MSGQ_CREATE, _msgq_create);
    syscall_register(MSGQ_SEND, _msgq_send);
    syscall_register(MSGQ_RECV, _msgq_recv);
    syscall_register(EVENT_CREATE, _event_create);
    syscall_register(EVENT_WAIT, _event_wait);
    syscall_register(EVENT_SET, _event_set);
}