#include "os.h"

extern int atomic_swap(spinlock_t *lock, int i);

/* for device*/
void lock_init(spinlock_t *lock)
{
  lock->locked = 0;
}

void lock_acquire(spinlock_t *lock)
{
  for (;;)
  {
    if (!atomic_swap(lock,1))
    {
      break;
    }
  }
}

void lock_free(spinlock_t *lock)
{
  //lock->locked = 0;
  for (;;)
  {
    if (atomic_swap(lock,0))
    {
      break;
    }
  }
}

reg_t baseLock()
{
	reg_t status;
	status = r_mstatus();
	w_mstatus(status & ~MSTATUS_MIE); //直接關閉中斷開關
	return status;
}

void baseUnLock(reg_t status)
{
	w_mstatus(status);
	//kprintf("%s call unlock set=%x \n", status, __FUNCTION__);
}
