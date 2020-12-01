**只有状态为runnable的进程才能够进入运行队列。当前正在运行的进程并不会在运行队列中**



#### 练习0：填写已有实验

本实验依赖实验1/2/3/4/5。请把你做的实验2/3/4/5的代码填入本实验中代码中有“LAB1”/“LAB2”/“LAB3”/“LAB4”“LAB5”的注释相应部分。并确保编译通过。注意：为了能够正确执行lab6的测试应用程序，可能需对已完成的实验1/2/3/4/5的代码进行进一步改进。

#### 练习1:  使用 Round Robin（时间片轮转算法） 调度算法（不需要编码）

完成练习0后，建议大家比较一下（可用kdiff3等文件比较软件）个人完成的lab5和练习0完成后的刚修改的lab6之间的区别，分析了解lab6采用RR调度算法后的执行过程。执行make grade，大部分测试用例应该通过。但执行priority.c应该过不去。

注意：

​	用run_link作为PCB的参与就绪队列节点，因为其就只有前后指针，即8字节，轻量；

​	PCB中有队列rq，由于运行的是该进程，因此应将该进程的rq替换为全局的rq



首先使用diff工具对lab5和lab进行比较，发现主要的区别有：

- PCT中增加了三个与stride调度算法相关的成员变量，以及增加了对应的初始化过程；
- 新增了斜堆数据结构的实现；
- 新增了默认的调度算法Round Robin的实现，具体为调用sched_class_*等一系列函数之后，进一步调用调度器sched_class中封装的函数，默认该调度器为Round Robin调度器，这是在default_sched.*中定义的；
- 新增了set_priority，get_time的系统调用；

proc_Struct增加参数

```
    struct run_queue *rq;                       // running queue contains Process
    list_entry_t run_link;                      // the entry linked in run queue
    int time_slice;                             // time slice for occupying the CPU
    skew_heap_entry_t lab6_run_pool;            // FOR LAB6 ONLY: the entry in the run pool
    uint32_t lab6_stride;                       // FOR LAB6 ONLY: the current stride of the process 
    uint32_t lab6_priority;                     // FOR LAB6 ONLY: the priority of process, set by lab6_set_priority(uint32_t)
```

​	增加了一个成员变量**time_slice**，用来记录进程当前的可运行时间片段。这是由于RR调度算法需要考虑执行进程的运行时间不能太长。在每个timer到时的时候，操作系统会递减当前执行进程的time_slice，当time_slice为0时，就意味着这个进程运行了一段时间（这个时间片段称为进程的时间片），需要把CPU让给其他进程执行，于是操作系统就需要让此进程重新回到rq的队列尾，且重置此进程的时间片为就绪队列的成员变量最大时间片max_time_slice值，然后再从rq的队列头取出一个新的进程执行。

进程队列

```
struct run_queue {
    list_entry_t run_list;//其运行队列的哨兵结构，可以看作是队列头和尾
    unsigned int proc_num;//表示其内部的进程总数
    int max_time_slice;//每个进程一轮占用的最多时间片
    // For LAB6 ONLY
    skew_heap_entry_t *lab6_run_pool;//优先队列形式的进程容器，只在 LAB6 中使用
};
```

请在实验报告中完成：

- 请理解并分析sched_calss中各个函数指针的用法，并接合Round Robin 调度算法描ucore的调度执行过程

  Round Robin 调度算法的调度思想是让所有 runnable态的进程分时轮流使用CPU时间。Round Robin 调度器维护当前 runnable进程的有序运行队列。当前进程的时间片用完之后，调度器将当前进程放置到运行队列的尾部，再从其头部取出进程进行调度。
  在这个理解的基础上，我们来分析算法的具体实现。
  这里Round Robin 调度算法的主要实现在default sched.c之中，源码如下：

RRinit函数：这个函数被封装为schedinit函数，

用于调度算法的初始化，使用grep命令可以知道，

该函数仅在ucore入口的init.c里面被调用进行初始化

```
static void
RR_init(struct run_queue *rq) {
list_init(&(rq->run_list));//初始化进程队列
rq->proc_num = 0;//初始化进程数为0
}
```

RR_enqueue的函数实现如下表所示。即把某进程的进程控制块指针放入到rq队列末尾，且如果进程控制块的时间片为0，则需要把它重置为rq成员变量max_time_slice。这表示如果进程在当前的执行时间片已经用完，需要等到下一次有机会运行时，才能再执行一段时间。

```
static void
RR_enqueue(struct run_queue *rq, struct proc_struct *proc) {
    assert(list_empty(&(proc->run_link)));
    list_add_before(&(rq->run_list), &(proc->run_link));
    if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
        proc->time_slice = rq->max_time_slice;
    }
    proc->rq = rq;
    rq->proc_num ++;
}
```

//RRenqueue函数：该函数的功能 为将指定的进程的状态置成RUNNABLE之后，放入调用算法中的可执行队列中，被封装成sched_class_enqueue函数，可以发现这个函数仅在 wakeup_proc和schedule函数中被调用，前者为将某个不是RUNNABLE的进程加入可执行队列，而后者是将正在执行的进程换出到可执行队列中去

---------------------

RR_dequeue的函数实现如下表所示。即把就绪进程队列rq的进程控制块指针的队列元素删除，并把表示就绪进程个数的proc_num减一。

```
//在schedule调用
static void
RR_dequeue(struct run_queue *rq, struct proc_struct *proc) {
    assert(!list_empty(&(proc->run_link)) && proc->rq == rq);
    list_del_init(&(proc->run_link));//就绪进程队列rq的进程控制块指针的队列元素删除
    rq->proc_num--;
}
```

RR_pick_next的函数实现如下表所示。即选取就绪进程队列rq中的队头队列元素，并把队列元素转换成进程控制块指针。

```
//在schedule调用
static struct proc_struct *
RR_pick_next(struct run_queue *rq) {
    list_entry_t *le = list_next(&(rq->run_list));
    if (le != &(rq->run_list)) {
    	//le2proc就是在通过就绪队列的指针le，在PCB中找到对应元素run_link，即某个run_list的入口
        return le2proc(le, run_link);//该函数通过偏移量逆向找到对因某个 run_list的 struct proc_struct。
    }
    return NULL;
}
```

le2proc函数

```
#define le2proc(le, member)         \
    to_struct((le), struct proc_struct, member)
    
/* *
 * to_struct - get the struct from a ptr
 * @ptr:    a struct pointer of member
 * @type:   the type of the struct this is embedded in
 * @member: the name of the member within the struct
 * */
#define to_struct(ptr, type, member)                               \
    ((type *)((char *)(ptr) - offsetof(type, member)))
    
/* Return the offset of 'member' relative to the beginning of a struct type */
#define offsetof(type, member)                                      \
    ((size_t)(&((type *)0)->member))
```

参数ptr 是待转换的地址，它属于某个结构体中某个成员的地址，

参数member是ptr所在结构体中对应地址的成员名字，也就是说参数member是个字符串，

参数type 是ptr所属的结构体的类型。

​	宏 le2proc 的作用是将指针ptr 转换成type 类型的指针，其原理是用
ptr的地址减去member在结构体type 中的偏移量，此地址差便是结构体type 的起始地址，最后再将此地址差转换为struct_type 指针类型。这里涉及到了另外一个宏offsetof。

​	宏 offsetof核心代码“ ((size_t)(&((type *)0)->member)) ”则为结构体成员member 在结构体中的偏移量。

​	因此：

​	宏le2proc 的原理，它将转换分为两步。
( 1) 用结构体成员的地址减去成员在结构体中的偏移量，先获取到结构体起始地址。
(2 ）再通过强制类型转换将第1 步中的地址转换成结构体类型。

```
思考：
一般的转型转换，只是改变了数据类型，并不改变地址，不同的数据类型仅仅是告诉编译器在同一地址处连续获取多少宇节的数据。比如：

​```
int four bytes = 0x12345678;
char one_bytes = (char) four_bytes;
​```

	Intel 是小端字节序， 因此单字节变量one_bytes 的值为0x78。

	我们这里要做的转换，不仅包括类型，还涉及到地址的转换。从队列中弹出的结点元素并不能直接用，因为咱们链表中的结点并不是PCB ，而是PCB 中的run_link，需要将它们转换成所在的PCB 的地址。比如咱们的办公地点是某大厦七层，有人要给咱们寄快递，咱们不能再告诉人家楼层了，必须要告诉人家大厦的地址。这种由楼层地址到大厦地址的转换就类似由以上两个tag 的地址到PCB 的地址转换。
```

​	**RR_proc_tick**的函数实现如下表所示。即每次timer到时后，trap函数将会间接调用此函数来把当前执行进程的时间片time_slice减一。如果time_slice降到零，则设置此进程成员变量need_resched标识为1，这样在下一次中断来后执行trap函数时，会由于当前进程程成员变量need_resched标识为1而执行schedule函数，从而把当前执行进程放回就绪队列末尾，而从就绪队列头取出在就绪队列上等待时间最久的那个就绪进程执行。

```
static void
RR_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
    if (proc->time_slice > 0) {
        proc->time_slice --;
    }
    if (proc->time_slice == 0) {
        proc->need_resched = 1;
    }
}
```

绑定

```
struct sched_class default_sched_class = {
    .name = "RR_scheduler",
    .init = RR_init,
    .enqueue = RR_enqueue,
    .dequeue = RR_dequeue,
    .pick_next = RR_pick_next,
    .proc_tick = RR_proc_tick,
};
```

接下来我们结合具体算法来描述一下**ucore调度执行过程**：

1. 在ucore中调用调度器的主体函数（不包括init，proc_tick）的代码仅存在在wakeup_proc和schedule，前者的作用在于将**某一个指定进程**放入可执行进程队列中，后者在于将**当前执行的进程**放入可执行队列中，然后将队列中选择的**下一个执行的进程取出**执行；
2. 当需要将某一个进程加入就绪进程队列中，则需要将这个进程的能够使用的时间片进行初始化max，然后将其插入到使用链表组织的队列的队尾；这就是具体的Round-Robin enqueue函数的实现；
3. 当需要将某一个进程从就绪队列中取出的时候，只需要将其直接删除即可；
4. 当需要取出执行的下一个进程的时候，只需要将就绪队列的队头取出即可，再通过le2proc转换出PCB；
5. 每当出现一个时钟中断，则会将当前执行的进程的剩余可执行时间减1，一旦减到了0，则将其标记为可以被调度的，这样就会调用schedule函数将这个进程切换出去；

-----------------------

- 请在实验报告中简要说明如何设计实现”多级反馈队列调度算法“，给出概要设计，鼓励给出详细设计

设计如下：

1. 在proc_struct中添加总共N个多级反馈队列的入口，每个队列都有着各自的优先级，编号越大的队列优先级约低，并且优先级越低的队列上时间片的长度越大，为其上一个优先级队列的两倍；并且在PCB中记录当前进程所处的队列的优先级；

2. 处理调度算法初始化的时候需要同时对N个队列进行初始化；

3. 在处理将进程加入到就绪进程集合的时候，观察这个进程的时间片有没有使用完，如果使用完了，就将所在队列的优先级调低，加入到优先级低1级的队列中去，如果没有使用完时间片，则加入到当前优先级的队列中去；

4. 在同一个优先级的队列内使用时间片轮转算法；

5. 在选择下一个执行的进程的时候，有限考虑高优先级的队列中是否存在任务，如果不存在才转而寻找较低优先级的队列；（有可能导致饥饿）

   ```
   进程饥饿，即为Starvation，指当等待时间给进程推进和响应带来明显影响称为进程饥饿。当饥饿到一定程度的进程在等待到即使完成也无实际意义的时候称为饥饿死亡。
   ```

6. 从就绪进程集合中删除某一个进程就只需要在对应队列中删除即可；

7. 处理时间中断的函数不需要改变；至此完成了多级反馈队列调度算法的具体设计；

![这里写图片描述](image/SouthEast)





#### 练习2: 实现 Stride Scheduling 调度算法（需要编码）

首先需要换掉RR调度器的实现，即用default_sched_stride_c覆盖default_sched.c。然后根据此文件和后续文档对Stride度器的相关描述，完成Stride调度算法的实现。

后面的实验文档部分给出了Stride调度算法的大体描述。这里给出Stride调度算法的一些相关的资料（目前网上中文的资料比较欠缺）。

- [strid-shed paper location1](http://wwwagss.informatik.uni-kl.de/Projekte/Squirrel/stride/node3.html)
- [strid-shed paper location2](http://citeseerx.ist.psu.edu/viewdoc/summary?doi=10.1.1.138.3502&rank=1)
- 也可GOOGLE “Stride Scheduling” 来查找相关资料

执行：make grade。如果所显示的应用程序检测都输出ok，则基本正确。如果只是priority.c过不去，可执行 make run-priority 命令来单独调试它。大致执行结果可看附录。（ 使用的是 qemu-1.0.1 ）。

请在实验报告中简要说明你的设计实现过程。

-----------

首先描述一下什么是Stride Scheduling调度算法，通过为每个进程设置优先级，来决定调度顺序．大致算法是这样的：

1. 维护两个变量，一个是stride,一个是pass．每次进程被调度的时候，stride+=pass;

2. 调度的时候取出当前最小的stride，作为获得cpu的对象．

   关于pass的值，等于BIG_STRIDE/优先级,所以我们通过设置优先级，可以给该进程设置好对应的步长.　根据这两个公式，我们可以知道优先级越高，越容易被调度．

3. 如何维护这个最小值的呢？我们知道通常面对动态提取最值问题，都是采用堆这种结构，于是ucore为我们维护了一个优先级队列（利用堆的思想）．利用这个结构，我们可以达到，每次更新数据都是log(n).

4. 如何避免stride溢出的问题．由于stride是不断增长的，当溢出的时候，便又是从小值开始，如果是这样的话，这意味着更容易被调度，从而会导致错误的调度结果．于是ucore巧妙的利用了整数在机器中的表示，将stride设置成无符号的类型，并且将BIG_STRIDE设置成有符号整数的最大值，通过指定priority大于1，可以将两个进程的stride之差锁定小于BIG_STRIDE．然后利用：

   ​	(int32_t)(p->lab6_stride - q->lab6_stride) > 0

   ​	便可正确的判断出两进程的stride真实大小关系．

--------------

关于Stride Scheduling 调度算法，经过查阅资料和实验指导书，我们可以简单的把思想归结如下：

1. 为每个runnable的进程设置一个当前状态stride，表示该进程当前的调度权。另外定义其对应的pass值，表示对应进程在调度后，stride需要进行的累加值。
2. 每次需要调度时，从当前 runnable态的进程中选择 stride最小的进程调度。对于获得调度的进程P，将对应的stride加上其对应的步长pass（只与进程的优先权有关系）。
3. 在一段固定的时间之后，回到步骤2，重新调度当前 stride最小的进程。

----------------



```
/* The compare function for two skew_heap_node_t's and the
 * corresponding procs
 * 优先级队列的比较函数*/
static int
proc_stride_comp_f(void *a, void *b)
{
     struct proc_struct *p = le2proc(a, lab6_run_pool);//run_pool即为队列池
     struct proc_struct *q = le2proc(b, lab6_run_pool);
     int32_t c = p->lab6_stride - q->lab6_stride;
     if (c > 0) return 1;
     else if (c == 0) return 0;
     else return -1;
}

/*
 * stride_init initializes the run-queue rq with correct assignment for
 * member variables, including:
 *
 *   - run_list: should be a empty list after initialization.
 *   - lab6_run_pool: NULL
 *   - proc_num: 0
 *   - max_time_slice: no need here, the variable would be assigned by the caller.
 *
 * hint: see libs/list.h for routines of the list structures.
 */
static void
stride_init(struct run_queue *rq) {
     /* LAB6: YOUR CODE 
      * (1) init the ready process list: rq->run_list
      * (2) init the run pool: rq->lab6_run_pool
      * (3) set number of process: rq->proc_num to 0       
      */
	list_init(&(rq->run_list));
	rq->lab6_run_pool = NULL;
	rq->proc_num = 0;
}

/*
 * stride_enqueue inserts the process ``proc'' into the run-queue
 * ``rq''. The procedure should verify/initialize the relevant members
 * of ``proc'', and then put the ``lab6_run_pool'' node into the
 * queue(since we use priority queue here). The procedure should also
 * update the meta date in ``rq'' structure.
 *
 * proc->time_slice denotes the time slices allocation for the
 * process, which should set to rq->max_time_slice.
 * 
 * hint: see libs/skew_heap.h for routines of the priority
 * queue structures.
 */
static void
stride_enqueue(struct run_queue *rq, struct proc_struct *proc) {
     /* LAB6: YOUR CODE 
      * (1) insert the proc into rq correctly
      * NOTICE: you can use skew_heap or list. Important functions
      *         skew_heap_insert: insert a entry into skew_heap
      *         list_add_before: insert  a entry into the last of list   
      * (2) recalculate proc->time_slice
      * (3) set proc->rq pointer to rq
      * (4) increase rq->proc_num
      */
    //assert(list_empty(&(proc->run_link)));
    rq->lab6_run_pool = skew_heap_insert(rq->lab6_run_pool,&(proc->lab6_run_pool), proc_stride_comp_f);
    if (proc->time_slice == 0 || proc->time_slice > rq->max_time_slice) {
        proc->time_slice = rq->max_time_slice;//即为到分片时间了，需要移到等待队列
    }
    proc->rq = rq;//由于运行的是该进程，因此应将该进程的rq替换
    rq->proc_num++;
}

/*
 * stride_dequeue removes the process ``proc'' from the run-queue
 * ``rq'', the operation would be finished by the skew_heap_remove
 * operations. Remember to update the ``rq'' structure.
 *
 * hint: see libs/skew_heap.h for routines of the priority
 * queue structures.
 */
static void
stride_dequeue(struct run_queue *rq, struct proc_struct *proc) {
     /* LAB6: YOUR CODE 
      * (1) remove the proc from rq correctly
      * NOTICE: you can use skew_heap or list. Important functions
      *         skew_heap_remove: remove a entry from skew_heap
      *         list_del_init: remove a entry from the  list
      */
	//assert(!list_empty(&(proc->run_link)) && proc->rq == rq);
	rq->lab6_run_pool = skew_heap_remove(rq->lab6_run_pool,&(proc->lab6_run_pool), proc_stride_comp_f);
	rq->proc_num--;
}
/*
 * stride_pick_next pick the element from the ``run-queue'', with the
 * minimum value of stride, and returns the corresponding process
 * pointer. The process pointer would be calculated by macro le2proc,
 * see kern/process/proc.h for definition. Return NULL if
 * there is no process in the queue.
 *
 * When one proc structure is selected, remember to update the stride
 * property of the proc. (stride += BIG_STRIDE / priority)
 *
 * hint: see libs/skew_heap.h for routines of the priority
 * queue structures.
 */
//选择堆的根节点即可
static struct proc_struct *
stride_pick_next(struct run_queue *rq) {
     /* LAB6: YOUR CODE 
      * (1) get a  proc_struct pointer p  with the minimum value of stride
             (1.1) If using skew_heap, we can use le2proc get the p from rq->lab6_run_pool
             (1.2) If using list, we have to search list to find the p with minimum stride value
      * (2) update p;s stride value: p->lab6_stride
      * (3) return p
      */
	if(rq->lab6_run_pool == NULL)	return NULL;
	struct proc_struct *p = le2proc(rq->lab6_run_pool,lab6_run_pool);//选择堆顶元素
	p->lab6_stride += BIG_STRIDE/p->lab6_priority;
	return p;
}

/*
 * stride_proc_tick works with the tick event of current process. You
 * should check whether the time slices for current process is
 * exhausted and update the proc struct ``proc''. proc->time_slice
 * denotes the time slices left for current
 * process. proc->need_resched is the flag variable for process
 * switching.
 */
static void
stride_proc_tick(struct run_queue *rq, struct proc_struct *proc) {
     /* LAB6: YOUR CODE */
	if(proc->time_slice > 0)
		proc->time_slice--;
	if (proc->time_slice == 0) {//时间片用尽
	        proc->need_resched = 1;
	    }
}

struct sched_class default_sched_class = {
     .name = "stride_scheduler",
     .init = stride_init,
     .enqueue = stride_enqueue,
     .dequeue = stride_dequeue,
     .pick_next = stride_pick_next,
     .proc_tick = stride_proc_tick,
};

```



需要保证 BigStride<2^(32-1)

```
注：BIG_STRIDE的值是怎么来的？
```

 1. 对于有符号数：数字总是在循环的变化。如从最大2147483647，再加一后就变成了最小-2147483648。即循环的顺序是：

    0— 2147483647—  -2147483648— 0。

    规律：

    SHRT_MAX+1 == SHRT_MIN

	2. 下面解释一下如何对两个进程的调度权进行比较proc_stride_comp_f及BIG_STRIDE = 0x7FFFFFFF的原因：
    从定义上可以知道，调度权被定义为32位无符号数，而随着每次调度后增加BIG_STRIDE / priority最终会导致溢出，正确获得两个进程调度权的真实大小很关键，利用函数proc_stride_comp_f实现，本质上是直接对两个32位无符号的调度权求差，并化为32位有符号数与0进行比较，来确定两个进程调度权的大小

    int32_t c = p->lab6_stride - q->lab6_stride;
    if (c > 0) return 1;       //p->lab6_stride > q->lab6_stride
    else if (c == 0) return 0; //p->lab6_stride == q->lab6_stride
    else return -1;            //p->lab6_stride < q->lab6_stride

        1
        2
        3
        4

    假设存在两个32位无符号数a,b分别表示调度权，初始时a=b，定义步长s = BIG_STRIDE / priority <=S（S为最大步进）
    （1）调度b后b+s=B，若B未溢出，且a-B=a-(b+s)=-s<0而不会下溢有符号数，则s<2^31，此时a-B<0成立
    （2）调度b后b+s=B，若B溢出，此时B溢出意味着b+s>2^32，B=b+s-2^32，由（1）可知s<2^31那么必有a=b>2^31，为了a-B<0即a-B的值上溢有符号数，需要使a-(b+s-2^32)>=2^31则s<=2^31

    综上可见s<2^31即最大步进S的精确值为S=2^31-1=0x7FFFFFFF

#### 扩展练习 Challenge 1 ：实现 Linux 的 CFS 调度算法

在ucore的调度器框架下实现下Linux的CFS调度算法。可阅读相关Linux内核书籍或查询网上资料，可了解CFS的细节，然后大致实现在ucore中。

#### 扩展练习 Challenge 2 ：在ucore上实现尽可能多的各种基本调度算法(FIFO, SJF,…)，并设计各种测试用例，能够定量地分析出各种调度算法在各种指标上的差异，说明调度算法的适用范围。