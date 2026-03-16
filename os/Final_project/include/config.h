#define MAX_USER_TASKS      256
#define SYS_TASK_NUM        1
#define SYS_STACK_SIZE      256
#define USER_STACK_SIZE     1024
#define PRIO_LEVEL          256
#define TICK_PER_SECOND     50
/* interval ~= 1s */
#define SYSTEM_TICK CLINT_TIMEBASE_FREQ/TICK_PER_SECOND
#define MAX_TIMERS          256 //08
#define MAX_SEM             256 //09 sem
#define MAX_SEM_VALUE       1024 //09 sem
#define MAX_EVENT           256
#define MAX_MBOXS           256
#define MAX_EXTERNAL_INT    127
#define MAX_SYSCALL         256