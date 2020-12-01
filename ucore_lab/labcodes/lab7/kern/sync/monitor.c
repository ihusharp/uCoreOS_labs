#include <stdio.h>
#include <monitor.h>
#include <kmalloc.h>
#include <assert.h>


// Initialize monitor.
void     
monitor_init (monitor_t * mtp, size_t num_cv) {
    int i;
    assert(num_cv>0);
    mtp->next_count = 0;//睡在signal进程数为0
    mtp->cv = NULL;
    sem_init(&(mtp->mutex), 1); //unlocked,1表示未上锁
    sem_init(&(mtp->next), 0);//条件同步信量
    mtp->cv =(condvar_t *) kmalloc(sizeof(condvar_t)*num_cv);//分配一内核空间来放置条件变量
    assert(mtp->cv!=NULL);
    for(i=0; i<num_cv; i++){
        mtp->cv[i].count=0;
        sem_init(&(mtp->cv[i].sem),0);
        mtp->cv[i].owner=mtp;
    }
}

// Unlock one of threads waiting on the condition variable. 
void 
cond_signal (condvar_t *cvp) {
   //LAB7 EXERCISE1: YOUR CODE
   cprintf("cond_signal begin: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
   if(cvp->count > 0){//那么就说明存在需要唤醒的由于cv而阻塞的进程
	   cvp->owner->next_count++;//这说明为了唤醒的进程，而睡眠了。应当写在前面，因为马上进入up函数，此up函数表示对cv阻塞的进程进行唤醒
	   up(&(cvp->sem));//唤醒条件变量不足而睡眠的进程
	   //现在需要将自己阻塞
	   down(&(cvp->owner->next));
	   cvp->owner->next_count--;//说明down操作已经被唤醒
   }
  /*
   *      cond_signal(cv) {
   *          if(cv.count>0) {
   *             mt.next_count ++;
   *             signal(cv.sem);
   *             wait(mt.next);
   *             mt.next_count--;
   *          }
   *       }
   */
   cprintf("cond_signal end: cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}

// Suspend calling thread on a condition variable waiting for condition Atomically unlocks 
// mutex and suspends calling thread on conditional variable after waking up locks mutex. Notice: mp is mutex semaphore for monitor's procedures
void
cond_wait (condvar_t *cvp) {
    //LAB7 EXERCISE1: YOUR CODE
    cprintf("cond_wait begin:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
    cvp->count++;
    if(cvp->owner->next_count>0){
    	up(&(cvp->owner->next));//唤醒next进程
    }else{//说明没有由于唤醒A而睡眠的，因此进行释放锁
    	up(&(cvp->owner->mutex));
    }
    //由于没有释放锁，即存在由于唤醒A而睡眠的，现在需要将A睡眠，然后将B唤醒
    //因为down函数会进行调度，而up只是将其从sem等待队列中删除
    down(&(cvp->owner));
    cvp->count--;//表示睡眠完毕，已经苏醒，因此需要--
    /*
    *         cv.count ++;
    *         if(mt.next_count>0)
    *            signal(mt.next)
    *         else
    *            signal(mt.mutex);
    *         wait(cv.sem);
    *         cv.count --;
    */
    cprintf("cond_wait end:  cvp %x, cvp->count %d, cvp->owner->next_count %d\n", cvp, cvp->count, cvp->owner->next_count);
}
