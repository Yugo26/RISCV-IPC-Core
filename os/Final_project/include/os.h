#ifndef __OS_H__
#define __OS_H__

#include "types.h"
#include "config.h"
#include "platform.h"
#include "task.h"
#include "list.h"
#include "riscv.h"
#include "softtimer.h"
#include "ipc.h"
//#include "message.h"
#include "device.h"
#include "marco.h"
#include "usyscall.h"

#include <stdarg.h>



/* uart */
err_t waitRxData(int timeout);
deviceCB_t* serial_init();

/* printf */
int     kprintf(const char* s, ...);
int     lockfree_printf(const char* s, ...);
void    panic(char *s);

/* libc */
void *  memset(void *ptr, int value, size_t num);
char *  memcpy(void* dest,const void* src, size_t num);
int     strcmp(const char *s1, const char *s2);

/* lock */
reg_t   baseLock();
void    baseUnLock(reg_t);
void    lock_init(spinlock_t *lock);
void    lock_acquire(spinlock_t *lock);
void    lock_free(spinlock_t *lock);

/* memory management */
void    *page_alloc(int npages);
void    page_free(void *p);
void    *malloc(size_t size);
void    free(void *);

/* task*/
void    readyQ_init();
void    InitTCBList(void);
taskCB_t * task_create(const char *name,
                  void (*taskFunc)(void *parameter),
                  void       *parameter,
                  uint32_t    stack_size,
                  uint16_t    priority,
                  uint32_t    ticks);
void    task_startup(taskCB_t * ptcb);
err_t   task_resume(taskCB_t *ptcb);
err_t   task_suspend(taskCB_t * ptcb);
err_t   task_yield(void);
taskCB_t *getCurrentTask();
char    *getCurrentTaskName();
void    taskDelay(uint32_t);
err_t   idleTask_init();


/* sched */
void    sched_init();
void    schedule();

/* trap */
void    trap_init();
reg_t   trap_handler(reg_t epc, reg_t cause, ctx_t *ctx);

/* timer */
void    timer_load(int interval);
void    timer_init();
void    timer_handler();
uint64_t getTicks();

/*softtimer*/
void    softTimer_init();
err_t   createTimer(uint8_t timerType,  
                  uint32_t timerCount,
                  uint32_t timerReload,
                  void(*callback)(void *parameter),
                  void *parameter
                  );
err_t   startTimer(uint16_t timerID);
err_t   stopTimer(uint16_t timerID);
err_t   delTimer(uint16_t timerID);
uint32_t    getCurTimerCnt(uint16_t timerID);
err_t   setCurTimerCnt(uint16_t timerID,
                     uint32_t timerCount,
                     uint32_t timerReload);
void    timerDispose(void);


/*IPC sem 09*/
err_t   createSem(uint16_t initCnt,uint16_t maxCnt,uint8_t sortType);
void    delSem(uint16_t semID);
/*
timeout = 0, try, -1: for ever
*/
err_t   sem_take(uint16_t semID, int timeout);
err_t   sem_trytake(uint16_t semID);
err_t   sem_release(uint16_t semID);

/* Mutex */
err_t   createMut(uint16_t initCnt,uint16_t maxCnt,uint8_t sortType);
void    delMut(uint16_t semID);
err_t   mut_take(uint16_t semID, int timeout);
err_t   mut_trytake(uint16_t semID);
err_t   mut_release(uint16_t semID);

/* IPC mbox 09*/
err_t   createMbox(uint8_t sortType);
void    delMbox(uint16_t mboxID);
err_t   postMbox(uint16_t mboxID, void *pmail);
void *  acceptMail(uint16_t mboxID, err_t *perr);
void *  waitMail(uint16_t mboxID, uint32_t timeout, err_t *perr);

/* message queue */
err_t MsgQCreate(uint32_t msgSize, uint32_t maxMsgs);
err_t MsgQSend(uint16_t id, void *pBuffer, uint32_t nBytes, int timeout);
err_t MsgQRecv(uint16_t id, void *pBuffer, uint32_t nBytes, int timeout);

/* event*/
err_t eventCreate();
err_t event_wait(uint32_t id, uint32_t waitBits, uint8_t mode, uint32_t timeout);
err_t event_set(uint32_t id, uint32_t setFlags);


/* user*/
void    loadTasks(void);

/* lib */
/* queue */
void    TaskToWait(list_t *plist, uint8_t sortType, taskCB_t *ptcb);
void    WaitTaskToRdy(list_t *plist);
err_t   AllWaitTaskToRdy(list_t *plist);


/*plic*/
void isrRegister(int irq, void(*isr)(void));
void externalInterrupt();
void plic_init(void);
int plic_claim(void);
void plic_complete(int irq);


/* device driver control*/
void deviceList_init();//call by kernel()
err_t device_register(deviceCB_t *dev, const char *name, uint16_t flags);
err_t device_unregister(deviceCB_t *dev);
deviceCB_t * device_find(const char *name);
deviceCB_t * device_create(int type, char *name);
void device_free(deviceCB_t *dev);
err_t device_init(deviceCB_t *dev);
err_t device_open(deviceCB_t *dev, uint16_t oflag);
err_t device_close(deviceCB_t *dev);
size_t device_read(deviceCB_t *dev,
                    uint16_t    pos,
                    void       *buffer,
                    size_t   size);
size_t device_write(deviceCB_t *dev,
                    uint16_t    pos,
                    const void *buffer,
                    size_t   size);
err_t device_control(deviceCB_t *dev, int cmd, void *arg);
err_t device_set_rxReady(deviceCB_t *dev,
                    err_t (*rx_ind)(size_t size));
err_t device_set_txComplete(deviceCB_t *dev,
                    err_t (*tx_done)(void *buffer));

            
/* drivers */
void drivers_init();
typedef struct console_dev {
    deviceCB_t *dev;
    spinlock_t lock;
} console_t;

/* syscall*/
void syscall_init();
void syscall_register(uint16_t num, void *(*sysfunc)(void *p));
void do_syscall(ctx_t *ctx);

#endif /* __OS_H__ */
