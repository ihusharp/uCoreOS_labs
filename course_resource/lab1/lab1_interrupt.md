在操作系统中，有三种特殊的中断事件。由CPU外部设备引起的外部事件如I/O中断、时钟中断、控制台中断等是异步产生的（即产生的时刻不确定），与CPU的执行无关，我们称之为异步中断(asynchronous interrupt)也称外部中断,简称中断(interrupt)。而把在CPU执行指令期间检测到不正常的或非法的条件(如除零错、地址访问越界)所引起的内部事件称作同步中断(synchronous interrupt)，也称内部中断，简称异常(exception)。把在程序中使用请求系统服务的系统调用而引发的事件，称作陷入中断(trap interrupt)，也称软中断(soft interrupt)，系统调用(system call)简称trap。在后续试验中会进一步讲解系统调用。

保护模式下，最多会存在256个Interrupt/Exception Vectors。范围[0，31]内的32个向量被异常Exception和NMI使用，但当前并非所有这32个向量都已经被使用，有几个当前没有被使用的，请不要擅自使用它们，它们被保留，以备将来可能增加新的Exception。范围[32，255]内的向量被保留给用户定义的Interrupts。Intel没有定义，也没有保留这些Interrupts。用户可以将它们用作外部I/O设备中断（8259A IRQ），或者系统调用（System Call 、Software Interrupts）等。

lidt完成idt的加载
------------------------------------------------
Interrupts/Exceptions应该使用Interrupt Gate和Trap Gate，它们之间的唯一区别就是：
当调用Interrupt Gate时，Interrupt会被CPU自动禁止；

而调用Trap Gate时，CPU则不会去禁止或打开中断，而是保留它原来的样子。

**CPU把中断（异常）号乘以8做为IDT的索引**

-------------------------------------

请完成编码工作和回答如下问题：

1. 中断描述符表（也可简称为保护模式下的中断向量表）中一个表项占多少字节？其中哪几位代表中断处理代码的入口？
2. 请编程完善kern/trap/trap.c中对中断向量表进行初始化的函数idt_init。在idt_init函数中，依次对所有中断入口进行初始化。使用mmu.h中的SETGATE宏，填充idt数组内容。每个中断的入口由tools/vectors.c生成，使用trap.c中声明的vectors数组即可。
3. 请编程完善trap.c中的中断处理函数trap，在对时钟中断进行处理的部分填写trap函数中处理时钟中断的部分，使操作系统每遇到100次时钟中断后，调用print_ticks子程序，向屏幕上打印一行文字”100 ticks”。
-------------------------------------------------------------
kern/mm/mmu.h
//  内存管理单元硬件 Memory Management Unit 	
	将线性地址映射为物理地址,包括EFLAGS寄存器等段定义
/* Gate descriptors for interrupts and traps */
struct gatedesc {
    unsigned gd_off_15_0 : 16; //0-15
   						       // low 16 bits of offset in segment
    unsigned gd_ss : 16;            // segment selector
    unsigned gd_args : 5;            // # args, 0 for interrupt/trap gates
    unsigned gd_rsv1 : 3;            // reserved(should be zero I guess)
    unsigned gd_type : 4;            // type(STS_{TG,IG32,TG32})
    unsigned gd_s : 1;                // must be 0 (system)
    unsigned gd_dpl : 2;            // descriptor(meaning new) privilege level
    unsigned gd_p : 1;                // Present
    unsigned gd_off_31_16 : 16;    //16-31
    							    // high bits of offset in segment
};*
共64个bit 即8个字节
offset占32位（分为前后）
段选择子占16位
属性信息占16位
	-中断描述符表（也可简称为保护模式下的中断向量表）中一个表项占多少字节？其中哪几位代表中断处理代码的入口？

##### 	选择子对GDT进行段描述符的选择得到base + idt表项内的offset

**【注意】除了系统调用中断(T_SYSCALL)使用陷阱门描述符且权限为用户态权限以外，其它中断均使用特权级(DPL)为０的中断门描述符，权限为内核态权限；而ucore的应用程序处于特权级３，需要采用｀int 0x80 指令操作（这种方式称为软中断，软件中断，Tra中断，在lab5会碰到）来发出系统调用请求，并要能实现从特权级３到特权级０的转换，所以系统调用中断(T_SYSCALL)所对应的中断门描述符中的特权级（DPL）需要设置为３。**

-----------------------------

2. 请编程完善kern/trap/trap.c中对中断向量表进行初始化的函数idt_init。在idt_init函数中，依次对所有中断入口进行初始化。使用mmu.h中的SETGATE宏，填充idt数组内容。每个中断的入口由tools/vectors.c生成，使用trap.c中声明的vectors数组即可。

------
使用mmu.h中的SETGATE宏，填充idt数组内容。
#define SETGATE(gate, istrap, sel, off, dpl) {            \
    (gate).gd_off_15_0 = (uint32_t)(off) & 0xffff;        \
    (gate).gd_ss = (sel);                                \
    (gate).gd_args = 0;                                    \
    (gate).gd_rsv1 = 0;                                    \
    (gate).gd_type = (istrap) ? STS_TG32 : STS_IG32;    \
    (gate).gd_s = 0;                                    \
    (gate).gd_dpl = (dpl);                                \
    (gate).gd_p = 1;                                    \
    (gate).gd_off_31_16 = (uint32_t)(off) >> 16;        \
}
由代码看出SETGATE本质是设置生成一个8字节的中断描述表项
    gate为中断描述符表项对应的数据结构，定义在mmu.h为struct gatedesc
    istrap标识是中断还是系统调用，唯一区别在于，中断会清空IF标志，不允许被打断
    sel与off分别为中断服务例程的代码段与偏移量，
    dpl为访问权限设为3

    sel:GD_KTEXT 定义在memlayout.h中，是表示全局描述符表中的内核代码段选择子
    off:offset就用__vector所规定的地址即可
   	设置自定义的陷阱中断T_SWITCH_TOK（用于用户态切换到内核态）和实现对自定义的陷阱中断T_SWITCH_TOK/T_SWITCH_TOU的中断处理例程，使得CPU能够在内核态和用户态之间切换。

------------------------

vectors中存储了中断处理程序的入口程序和入口地址，即该数组中第i个元素对应第i个中断向量的中断处理函数地址。vectors定义在vector.S文件中，通过一个工具程序vector.c生成。而且由vector.S文件开头可知，中断处理函数属于.text的内容。因此，中断处理函数的段选择子即.text的段选择子GD_KTEXT。从kern/mm/pmm.c可知.text的段基址为0
  │   │   ├── memlayout.h  // 操作系统有关段管理（段描述符编号、段号等）的一些宏定义

#define GD_KTEXT    ((SEG_KTEXT) << 3)        // kernel text
#define DPL_KERNEL    (0)
#define DPL_USER    (3)
#define T_SWITCH_TOK                121    // user/kernel switch
static struct gatedesc idt[256] = {{0}};
-------------------------------------------------------
请编程完善trap.c中的中断处理函数trap，在对时钟中断进行处理的部分填写trap函数中处理时钟中断的部分，使操作系统每遇到100次时钟中断后，调用print_ticks子程序，向屏幕上打印一行文字”100 ticks”。

ticks++;
if(ticks%TICK_NUM == 0){
   		print_ticks();
}

經過上面的魔鬼后,,,怀疑老师是让我们诱惑下去的心

-------------------------------------------------
这些问题问得很好。不仔细琢磨，是问不出来的。

 

Q（1） 为什么第二个参数是0（代表是trap而不是interrupt），trap和interrupt的区别是什么？如果只为trap建立了idt那interrupt的idt在哪里建立？

A：因为将来应用程序的系统调用需要通过trap gate来实现。 在ucore labs中,trap用来实现系统调用，interrupt用来实现对外设中断的处理等。trap和intr都在idt处要建立。trap gate与interrupt gate的唯一区别，是调用interrupt gate里的handler前会清EFLAGS的IF位（即关中断），而调用trap gate的handler时对IF位没有影响。

 

Q（2）第三个参数，用的是GD_KTEXT，是因为vector.S的开头是 “.text” 吗？ 为什么GD_KTEXT在memlayout.h中是一个常量？这个段的选择址在编译前就确定了吗？还是和bootloader协商过？

A：GD_KTEXT代表中断服务例程在内核代码段。vector.S的开头的 “.text” 是GNU assembly 语言中的一个表示：下面是代码 section。GD_KTEXT=8, 8是内核代码段的选择子，根据这个值（即index）可以定位到GDT的段描述符。这个段的选择子是ucore编码设定的，也可以理解在编译前就确定了。没有与bootloader协商，因为ucore调用gdt_init函数又初始化了一次GDT.

 

Q(3) 对于__vectors[]在vector.S中的定义，为什么每个idt的offset都要先pushl 0，再pushl相应的数？ 

A: 不是每个idt的offset都要先pushl 0，再pushl相应的数。因为有些异常会产生错误码（比如页访问错误异常），有些异常不产生，为了统一，对于不产生的会pushl 0。请仔细看看tools/vector.c （生成 vectors.S），应该就了解了。http://pdos.csail.mit.edu/6.828/2011/lec/x86_idt.pdf 给出了每个中断向量是否有错误码。

 

Q（4）在mmu.h的struct gatedesc的定义里，gd_dpl的位数是2，也就是说有4种不同的特权级。在memlayout.h中，DP_KERNEL和DP_USER分别是0和3，那1和2分别是什么？

A: Intel CPU设计时，认为设置4级特权级可以适应更多的应用方式。但当前的主流OS只用了两个特权级，内核态（0级）和用户态（3级）就满足了需求。1,2特权级一般就没啥人用了。

补充一下第一小问.

1. mmu.h中有说明, "istrap: 1 for a trap (= exception) gate, 0 for an interrupt gate", 所以0表示interrupt, 该问题便有错误.

2. vectors的256个元素中, 0~31是保留的, 用于处理异常和NMI(不可屏蔽中断); 32~255由用户定义, 可以是设备中断或系统调用.

3.  Interrupt Gate 和 Trap Gate,它们之间的唯一区别就是:当调用Interrupt Gate 时,Interrupt 会被 CPU 自动禁止;而调用 Trap Gate 时,CPU 则不会去禁止或打开中断,而是保留它原来的样子

4. 我认为在这里应该根据vectors中的具体类型(trap或interrupt)分别初始化. lab1统一将第二个参数设置为interrupt, 是因为lab1暂时不用处理trap, 所以进行了简化处理. 类似情形在ucore labs中时常出现, 所以以后的lab会时常补充修正之前lab的代码.