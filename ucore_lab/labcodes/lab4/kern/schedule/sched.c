#include <list.h>
#include <sync.h>
#include <proc.h>
#include <sched.h>
#include <assert.h>

void
wakeup_proc(struct proc_struct *proc) {
    assert(proc->state != PROC_ZOMBIE && proc->state != PROC_RUNNABLE);
    proc->state = PROC_RUNNABLE;
}

void
schedule(void) {
    bool intr_flag;
    list_entry_t *le, *last;
    struct proc_struct *next = NULL;
    local_intr_save(intr_flag);
    {
    	//1.设置当前内核线程 current->need_resched 为 0（即练习一中的PCB “是否需要调度”）;
        current->need_resched = 0;
        //proc_list --->the process set's list 如果是idleproc，那就调用进程表的首地址
        //否则就搞当前的进程位置
        last = (current == idleproc) ? &proc_list : &(current->list_link);
        le = last;
        do {
        	/*
        	 * 遍历进程hash队列，
        	 * 在proc_list 队列中查找下一个处于就绪态的线程或进程next;
        	 * （比如，这里有一句：next state=runnable）
        	 */
            if ((le = list_next(le)) != &proc_list) {
                next = le2proc(le, list_link);
                if (next->state == PROC_RUNNABLE) {
                    break;
                }
            }
        } while (le != last);
        //没有找到，那就将next = idleproc，继续等待
        if (next == NULL || next->state != PROC_RUNNABLE) {
            next = idleproc;
        }
        next->runs ++;//运行时间
        /*
         * 找到这样的进程后，就调用 proc_run函数，
         * 保存当前进程current的执行现场(进程上下文)，
         * 恢复新进程的执行现场，完成进程切换。
         */
        if (next != current) {
            proc_run(next);//proc_run函数，就可以跑当前被调度选出的进程，从runable状态正式开始运行。
        }
    }
    local_intr_restore(intr_flag);
}

