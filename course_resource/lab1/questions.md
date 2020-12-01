在一个段寄存器里面，会保存一个值叫段选择子
offset当是段机制下，是eip提供
当中断时，由IDT得到

实模式下段的基地址等于段寄存器的值得乘以16，保护模式下段的基地址在描述符表中给出，段选择子的高13位就是描述符表（2个，全局和局部）的索引号（0~8191）。 

------------------

对lab1中的bootmain.c中若干细节的疑惑
1 在readseg函数中va向下取整的疑惑

/* *
 * readseg - read &#64;count bytes at &#64;offset from kernel into virtual address &#64;va,
 * might copy more than asked.
 * */
static void
readseg(uintptr_t va, uint32_t count, uint32_t offset) {
    uintptr_t end_va = va + count;
    // round down to sector boundary
    va -= offset % SECTSIZE;
    // translate from bytes to sectors; kernel starts at sector 1
    uint32_t secno = (offset / SECTSIZE) + 1;
    // If this is too slow, we could read lots of sectors at a time.
    // We'd write more to memory than asked, but it doesn't matter --
    // we load in increasing order.
    for (; va < end_va; va += SECTSIZE, secno ++) {
        readsect((void *)va, secno);
 }
}

这里为什么va要向下取整？这样做难道不会不小心把新写的数据写到前面已经写好了的数据上而导致错误么？
2 readseg函数中读取扇区的方式

在代码注释中写说一次仅读取一个扇区速度较慢，可以一次读取多个扇区。那么这里提到的一次读取多个扇区的技术是如何实现的？DMA？还是简单在去除多次调用函数改为调用一次函数多次读取扇区？
3 bootmain函数最后bad的含义

bad:
    outw(0x8A00, 0x8A00);
    outw(0x8A00, 0x8E00);

bad的代码，是在加载bootloader后面扇区的kernel不满足elf文件的格式或者从kern_init函数返回后才会运行，但是对于这两行代码的具体含义我不太明白，向0x8A00端口传送0x8A00和0x8E00是什么意思，端口0x8A00又是什么？我查阅了互联网上的资料，只看到有说是端口0x8A00连接到模拟器，并会向模拟器转交控制，然后就没有了。我想了解得更具体一些，恳请老师同学们给我一点查阅的方向或者可以告诉我更详细的信息那就更好啦。谢谢！ 

A:

首先来看看 readseg 函数到底在什么情况下被调用。

在 bootmain 函数中读入了内核镜像 kernel 的 elf header :

readseg((uintptr_t)ELFHDR, SECTSIZE * 8, 0);

可见，从文件偏移量 [0x0,0x1000) 读取到虚拟内存地址 [0x10000,0x11000) 上。

其次，读入了各个 program header :

ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);
eph = ph + ELFHDR->e_phnum;
for (; ph < eph; ph ++) {
    readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);
}

我们可以在内核启动起来之后用和上面相同的方式输出传入 readseg 的参数：

在 kernel/init/init.c 中增加

void disk_test() {
    cprintf("hello world!, 0x%x\n", ELFHDR->e_magic);
    cprintf("phnum = %u\n", ELFHDR->e_phnum);
    struct proghdr *ph, *eph;
    ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);
    eph = ph + ELFHDR->e_phnum;
    for (; ph < eph; ph++) {
        cprintf("va = 0x%x memsz = 0x%x offset=0x%x\n",ph->p_va, ph->p_memsz, ph->p_offset);
    }
}

并在 kern_init 的开头就调用它。

运行后可以看到结果是：

hello world!, 0x464c457f
phnum = 3
va = 0x100000 memsz = 0xe310 offset=0x1000
va = 0x10f000 memsz = 0x1d20 offset=0x10000
va = 0x0 memsz = 0x0 offset=0x0

也就是一共有三个段，略去大小为 0 的第三个不看，前两个分别：

    从文件偏移量 [0x1000,0xf310) 读取到虚拟内存区间 [0x100000,0x10e310) ；
    从文件偏移量 [0x10000,0x11d20) 读取到虚拟内存区间 [0x10f000,0x110d20) 。

不必运行模拟器的情况下，我们还可以借助 objdump 来看看 kernel 中这些段更具体的信息：

$ objdump target/kernel -x

bin/kernel:     file format elf32-i386
bin/kernel
architecture: i386, flags 0x00000112:
EXEC_P, HAS_SYMS, D_PAGED
start address 0x001001f0

Program Header:
    LOAD off    0x00001000 vaddr 0x00100000 paddr 0x00100000 align 2**12
         filesz 0x0000e310 memsz 0x0000e310 flags r-x
    LOAD off    0x00010000 vaddr 0x0010f000 paddr 0x0010f000 align 2**12
         filesz 0x00000a16 memsz 0x00001d20 flags rw-
   STACK off    0x00000000 vaddr 0x00000000 paddr 0x00000000 align 2**4
         filesz 0x00000000 memsz 0x00000000 flags rwx

Sections:
Idx Name          Size      VMA       LMA       File off  Algn
  0 .text         00003599  00100000  00100000  00001000  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  1 .rodata       0000096c  001035a0  001035a0  000045a0  2**5
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  2 .stab         0000801d  00103f0c  00103f0c  00004f0c  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  3 .stabstr      000023e7  0010bf29  0010bf29  0000cf29  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  4 .data         00000a16  0010f000  0010f000  00010000  2**5
                  CONTENTS, ALLOC, LOAD, DATA
  5 .bss          00001300  0010fa20  0010fa20  00010a16  2**5
                  ALLOC
  6 .comment      0000002b  00000000  00000000  00010a16  2**0
                  CONTENTS, READONLY

仔细观察数据，发现

    第一个段是与 .text, .rodata, .stab, .stabstr 几个 Sections 对应的；
    第二个段是与 .data, .bss 对应的。

于是 readseg 的三次调用，其 va 分别为 0x10000, 0x100000, 0x10f000 ，都是 SECTSIZE 的倍数，实际上向下取整的语句并未发挥作用。也更不必担心覆盖数据的问题了。

事实上，只要保证这些段的开头位置与将要放在的起始虚拟地址均对齐于扇区大小 SECTSIZE ，那就几乎不会有问题。
2

static void
readsect(void *dst, uint32_t secno) {
    ...

    outb(0x1F2, 2);
    
    ...
    
    insl(0x1F0, dst, SECTSIZE / 2);
}
static void
readseg(uintptr_t va, uint32_t count, uint32_t offset) {
    ...
    for (; va < end_va; va += 1024, secno += 2) {
        readsect((void *)va, secno);
    }
}

也许可以通过将端口 0x1F2 对应的设备寄存器设为 2 来每次读入两个扇区。然而，在 bootmain.c 中做上述修改，发现 bootloader 的大小超过了 510 字节，违反了 BIOS 的限制。

话说回来，一次操作读写 1 个块还是 2 个块并不能影响磁盘的运行速度。在 bootmain.c 中这种通过 polling 循环等待磁盘操作结束再继续下面的流程的实现中，程序的总运行时间是一样的。

然而，如果 CPU 发出了I/O 请求后不是通过 polling 等待操作结束，而是去做别的事情(切换到别的进程)，并使用中断机制及时回到工作现场。那么越频繁的发起 I/O 请求会增加上下文切换的次数，降低 CPU 的使用效率。于是，一次操作多读写一些会比较好。

DMA 方面，需要模拟器提供的支持，暂时没有进行调研，可以留待后续补充。
3

个人觉得对于模拟器而言，这种致命情况直接 panic 或者直接死循环即可，并不清楚“把控制移交给模拟器”有何意义。

尝试了一下将那两行代码删掉，感觉删掉前后并没有什么变化，可能只有 while(1) 在起作用吧。
edit
·
thanks!

谢谢你的回答，非常详尽。下面我在说一些我的想法。
对于（1）来说，按照你的意思va的向下取整是作为一种冗余的保护措施存在，但是我觉得这种冗余的保护措施可能会导致数据覆盖的错误，甚至会导致进入ELFHDR->e_entry错误，这样的话直接删除不是更合适么？
对于（3）来说，我知道到bad的情况比较致命，但是我还是想搞清楚这个0x8a00端口的具体信息，可能我有点钻牛角尖了。

1

从实际应用的角度看，出于各种因素，通常会通过设置链接脚本保证各段满足对齐要求，通常是以一页为单位 (4096 字节)，则向下取整就完全没有必要了。

但是，也可以强行设置不满足对齐要求。在这种情况下，如果对于所有的段，其在文件中的位置和加载到虚拟内存之后的位置之间差值是一个定值，等于是整个文件整体平移到虚拟内存中。此时，向下取整不会产生问题。

但是如果既不满足对齐要求，上述的定值条件也不满足，向下取整确实会出现覆盖之前读入的数据的情况。此时我们只能开一块缓冲区存储从 IDE 中读到的数据，然后再将其中合适的部分写入到内存，而不是直接整块覆盖到内存。
3

这种写法应该来源于 MIT 的 xv6 ，在这里可以看到，解析 ELF 失败后使用汇编代码进行了一样的操作。在注释中提到了它是针对于 Bochs 模拟器起作用的，作用大概是进行调试。

Bochs模拟器是一个 i386 模拟器，在手册中找到了有关 0x8A00 端口的相关使用方法，也介绍了写入 0x8A00,0x8AE0 各是什么含义，有兴趣的话可以通过 Bochs 模拟器运行 uCore 看看会如何。

因此结论是，这两行代码对于 Qemu 来说并没有用(从 Qemu 源代码中也未找到对应的 PIO 映射)，它是在 Bochs 中调试用的。
helpful! 1
[Esoom]
Esoom
27 days ago

完全明白了，非常感谢！
helpful! 0 
----------------------------------------------------
在做lab1 练习6时，有几点疑问

 

1.为何extern uintptr_t __vectors[] 可以识别到在汇编文件vectors.S中定义的数组？

 

2.SETGATE的宏定义为 SETGATE(gate, istrap, sel, off, dpl) 

   为什么第三个参数填GD_KTEXT 就表示GDT的段选择子呢？

   GD_KTEXT这个宏的值是如何确定的？为何是8？

   为何__vectors[i]数组里的内容就是 中断服务程序的偏移地址OFFSET？

 

3. SETGATE(idt[T_SWITCH_TOK], 0, GD_KTEXT, __vectors[T_SWITCH_TOK], DPL_USER); 

    这个是必须的吗？没写怎么也能输出正确结果，不写的话对以后的实验会有影响吗？

 

4.struct gatedesc{} 该结构体有何用处？ 结构体数组idt如何被调用？

（虽然已经知道 中断发生后，会根据中断号查询IDT，但是在代码中如何体现？ 中断号-->IDT-->GDT-->基址+OFFSET=ISR入口地址 找到入口地址之前的函数具体如何调用的能否讲解？）

 

5.idt的起始地址为何是这样定义？

struct pseudodesc idt_pd = {
    sizeof(idt) - 1, (uintptr_t)idt
};

6.有时候流程理解了，但是代码中有很多数据结构不知道为什么是这么定义的，怎么办？

比如说上面提到的gatedesc 以及memlayout.h中的global segment numbers ,global descriptor numbers等宏定义

A:

1 汇编文件 vector.S 也进行了编译 生成了 vector.o 这和用c写没有区别

2 段选择子是 段描述符在 GDT 中的索引 之前我们看到第0个段是空段 第一个段是内核代码段 所以索引是 1*8

3 用于 lab1 challange 的实现 切换特权级

4 结构是便于编码，执行的时候会进入 Vector.S 中执行相应的ISR 也最后回调到 trap.c 中

5 加载idt 到idtr 中是 低地址是limit 高地址是base 同加载gdt  是一样的

-----------------------------------------------------------------
在练习六第二问中，需要利用vector填写idt的内容，利用了mmu.h中的SETGATE宏，但是自己实现的时候并不懂的怎么用这个宏填充idt，后来参考答案后，大概理解了一下，但是还是有很多不明白的地方，如GD_KTEXT是什么，如何判断idt的特权级等等，希望大神能讲解一下


Intel® 64 and IA-32 Architectures Software Developer’s Manual Vol.3 Section 6.11当中写出了IDT各个位的具体含义，参照struct gatedesc的定义可以弄明白各个域跟实际的位的对应关系，进而得知SETGATE里各条语句实际上的作用。这里我们用GD_KTEXT填充gate的gd_ss域，就是设置其Segment Selector为内核的代码段（一个Index为1，使用GDT，DPL为0的Segment Selector，也就是0x8）。

 

IDT的特权级在实际的中断处理流程中产生的作用可以参见Manual Vol.2 Section 3.2当中对INT n指令执行细节的描述，发现使用这一特权级是在判断是否gate的DPL大于等于当前CPL（否则将产生General Protection Fault），也就是说，IDT的DPL用于控制能从什么样的特权级的代码执行当中通过调用INT n跳转到该中断处理程序。

SETGATE宏的第二个参数istrap的设置

在给出的答案中，所有istrap全部设置为0. 而根据mmu.h中的定义，istrap用来区分异常和中断。

 

我以为合适的实现方式应该如下所示：

 

    int i;
    for (i = 0; i < 256; i ++) {
        if (i == T_SYSCALL) {
            SETGATE(idt[i], 1, GD_KTEXT, __vectors[i], DPL_USER)
        }
        else if (i < IRQ_OFFSET) {
            SETGATE(idt[i], 1, GD_KTEXT, __vectors[i], DPL_KERNEL)
        }
        else {
            SETGATE(idt[i], 0, GD_KTEXT, __vectors[i], DPL_KERNEL)
        }
    }

即应该区分异常和中断对istrap参数进行不同的设置。

而不是参考答案中的

    int i;
    for (i = 0; i < sizeof(idt) / sizeof(struct gatedesc); i ++) {
        SETGATE(idt[i], 0, GD_KTEXT, __vectors[i], DPL_KERNEL);
    }
  // set for switch from user to kernel
    SETGATE(idt[T_SWITCH_TOK], 0, GD_KTEXT, __vectors[T_SWITCH_TOK], DPL_USER);
    ---------------------------
如果采用老的实现的话，所有trap都会被当成interrupt处理。

 

trap和interrupt处理的不同之处在于interrupt gate会将IF flag清零而trap gate不会。

 

也就是说，在老的实现下，操作系统在处理trap的时候将无法响应maskable interrupt，这会导致功能和标准出现不同。

 

一个简单的例子就是，当我们的应用程序产生了一个trap的时候，我们本来可以通过键盘强制关闭该程序，但是此时键盘的中断信号无法被接收，必须等到trap处理完才能得到响应。

 

即便如此，在大多数情况，操作系统仍然可以正常完成其功能，而且用户很难发现有何不同。



-----------------------------------------

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

3. Interrupt Gate 和 Trap Gate,它们之间的唯一区别就是:当调用Interrupt Gate 时,Interrupt 会被 CPU 自动禁止;而调用 Trap Gate 时,CPU 则不会去禁止或打开中断,而是保留它原来的样子

4. 我认为在这里应该根据vectors中的具体类型(trap或interrupt)分别初始化. lab1统一将第二个参数设置为interrupt, 是因为lab1暂时不用处理trap, 所以进行了简化处理. 类似情形在ucore labs中时常出现, 所以以后的lab会时常补充修正之前lab的代码.