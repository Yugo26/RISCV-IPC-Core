#include "os.h"

int sR_sem;
int sR_mut;
uint16_t g_queue_ID;
int eventID;

void ipcTestInit() {
	sR_sem=(int) createSem(1,1,FIFO);
	sR_mut=(int) createMut(1,1,FIFO);
    g_queue_ID = (uint16_t) MsgQCreate(sizeof(int), 10);
    eventID= (int) eventCreate();
}


    
void shareRoutine(int data, int timeout)
{
	uint32_t p[]={sR_sem, timeout};
	DEBUG("Enter shareRoutine\n");
	if (u_sem_take(p)!=E_OK) 
	{
		DEBUG("sem Taking timeout\n");
	}
	//waiting for ever if wait 
	/*   this section is critical */
	DEBUG("Task %s: Enter critical\n", getCurrentTask()->name);
	DEBUG("waiting in SEM\n");
    delay(5);
	DEBUG("data is = %d\n",data);
	/* critical to here*/
	DEBUG("Task %s: Release sem\n",getCurrentTask()->name);
	u_sem_release(sR_sem);
}

void mutexRoutine(int data, int timeout){
	uint32_t p[]={sR_mut, timeout};
	DEBUG("Enter mutexRoutine\n");

	if(u_mut_take(p)!=E_OK){
		DEBUG("Mutex Taking timeout\n");
	}

	DEBUG("Task %s:Enter critical (Got Mutex)\n", getCurrentTask()->name);
	DEBUG("waiting in MUT\n");
	delay(5);
	DEBUG("data is = %d\n",data);
	DEBUG("Task %s: Release mutex\n", getCurrentTask()->name);
	u_mut_release(sR_mut);
}

void msgSendRoutine(uint16_t id, void *pBuffer, uint32_t nBytes, uint32_t timeout)
{
    uint32_t p[] = {(uint32_t)id, (uint32_t)pBuffer, nBytes, timeout};
    DEBUG("Enter msgSendRoutine\n");
    if (u_msgq_send(p) != E_OK) {
        DEBUG("Msg Send Failed / Timeout\n");
    } else {
        DEBUG("Msg Send Success\n");
    }
}
void msgRecvRoutine(uint16_t id, void *pBuffer, uint32_t nBytes, uint32_t timeout)
{
    uint32_t p[] = {(uint32_t)id, (uint32_t)pBuffer, nBytes, timeout};
    
    DEBUG("Enter msgRecvRoutine\n");
    if(u_msgq_recv(p) != E_OK) {
        DEBUG("Task %s: Recv Timeout (Queue Empty)\n", getCurrentTask()->name);
    } else {
        int *n = (int)&pBuffer;
        DEBUG("Task %s: GOT DATA = %d\n", getCurrentTask()->name, n);
    }
}

void eventWaitRoutine(uint32_t id, uint32_t waitBits, uint8_t mode, uint32_t timeout)
{
    uint32_t p[] = {id, waitBits, (uint32_t)mode, timeout};
    
    DEBUG("Task %s: Start Waiting Event (Bits: 0x%x)...\n", getCurrentTask()->name, waitBits);
    
    if(u_event_wait(p) != E_OK){
        DEBUG("Task %s: Wait Timeout or Error\n", getCurrentTask()->name);
    } else {
        DEBUG("Task %s: Got Event! condition met.\n", getCurrentTask()->name);
    }
}

void eventSetRoutine(uint32_t id, uint32_t setFlags)
{
    uint32_t p[] = {id, setFlags};
    
    DEBUG("Task %s: Setting Event (Flags: 0x%x)...\n", getCurrentTask()->name, setFlags);
    
    if(u_event_set(p) != E_OK){
        DEBUG("Task %s: Set ERROR\n", getCurrentTask()->name);
    } else{
        DEBUG("Task %s: Set Successfully\n", getCurrentTask()->name);
    }
}
/*void user_task_recursive(int data, int timeout)
{
    DEBUG("Recursive Task: Created!\n");

    uint32_t p_take[] = {sR_mut, timeout}; 
    
    int i = 0;
    while (1){
        DEBUG("\n=== Round %d Start ===\n", i++);

        DEBUG("[L1] Trying to take Mutex (Outer)...\n");
        if(u_mut_take(p_take) == E_OK) {
            DEBUG("[L1] Got Mutex! (LockCount = 1)\n");
            delay(2);

            DEBUG("    [L2] Trying to take Mutex AGAIN (Inner)...\n");
            
            if(u_mut_take(p_take) == E_OK) {
                DEBUG("    [L2] Got Mutex! RECURSIVE SUCCESS! (LockCount = 2)\n");
                delay(2);

                DEBUG("    [L2] Releasing Inner lock...\n");
                u_mut_release(sR_mut);
                
                DEBUG("    [L2] Released. (LockCount should be 1, still locked)\n");

            } else {
                DEBUG("    [L2] FAILED to take recursive lock! (Deadlock?)\n");
            }

            DEBUG("[L1] Releasing Outer lock...\n");
            u_mut_release(sR_mut);
            DEBUG("[L1] Released. (LockCount should be 0, Fully Free)\n");

        } else {
            DEBUG("[L1] Failed to take Mutex (Timeout)\n");
        }

        DEBUG("=== Round End (Free) ===\n");
        
    }
}*/