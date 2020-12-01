#include <defs.h>
#include <list.h>
#include <proc.h>
#include <assert.h>
#include <default_sched.h>

static void
RR_init(struct run_queue *rq) {
    list_init(&(rq->run_list));//初始化进程队列 run_list：其运行队列的哨兵结构，可以看作是队列头和尾
    rq->proc_num = 0;//初始化进程数为0
}

//在wakeup_proc和schedule调用
static void
RR_enqueue(struct run_queue *rq, struct proc_struct *proc) {
    assert(list_empty(&(proc->run_link)));
    list_add_before(&(rq->run_list), &(proc->run_link));//run_link放到run_list的末尾
    if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
        proc->time_slice = rq->max_time_slice;//即为到分片时间了，需要移到等待队列
    }
    proc->rq = rq;//由于运行的是该进程，因此应将该进程的rq替换
    rq->proc_num++;
}

//在schedule调用
static void
RR_dequeue(struct run_queue *rq, struct proc_struct *proc) {
    assert(!list_empty(&(proc->run_link)) && proc->rq == rq);
    list_del_init(&(proc->run_link));//就绪进程队列rq的进程控制块指针的队列元素删除
    rq->proc_num--;
}

//在schedule调用
static struct proc_struct *
RR_pick_next(struct run_queue *rq) {
    list_entry_t *le = list_next(&(rq->run_list));
    if (le != &(rq->run_list)) {
        return le2proc(le, run_link);//该函数通过偏移量逆向找到对因某个 run_link的 PCB。
    }
    return NULL;
}

static void
RR_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice > 0) {
        proc->time_slice --;
    }
    if (proc->time_slice == 0) {//时间片用尽
        proc->need_resched = 1;
    }
}

struct sched_class default_sched_class = {
    .name = "RR_scheduler",
    .init = RR_init,
    .enqueue = RR_enqueue,
    .dequeue = RR_dequeue,
    .pick_next = RR_pick_next,
    .proc_tick = RR_proc_tick,
};

