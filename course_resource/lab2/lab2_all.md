//free_area_t是一个数据结构，记录空闲页个数+链接各个空闲页base的作用
//Page是一个数据结构,包括每个页的属性及双向指针
//list_entry_t是一个数据结构，存储前后指针
--------------------------------------------------------
总结ucore.img的生成过程

    1.编译libs和kern目录下所有的.c和.S文件，生成.o文件，并链接得到bin/kernel文件
        编译boot目录下所有的.c和.S文件，生成.o文件，并链接得到bin/bootblock.out文件
    
    3.编译tools/sign.c文件，得到bin/sign文件
    
    4.利用bin/sign工具将bin/bootblock.out文件转化为512字节的bin/bootblock文件，并将bin/bootblock的最后两个字节设置为0x55AA
    
    5.为bin/ucore.img分配5000MB的内存空间，并将bin/bootblock复制到bin/ucore.img的第一个block，紧接着将bin/kernel复制到bin/ucore.img第二个block开始的位置
-------
到了32位的80386 CPU时代，内存空间扩大到了4G，多了段机制和页机制，但Intel依然很好地保证了80386向后兼容8086。地址空间的变化导致无法直接采用8086的启动约定。如果把BIOS启动固件编址在0xF000起始的64KB内存地址空间内，就会把整个物理内存地址空间隔离成不连续的两段，一段是0xF000以前的地址，一段是1MB以后的地址，这很不协调。为此，intel采用了一个折中的方案：默认将执行BIOS ROM编址在32位内存地址空间的最高端，即位于4GB地址的最后一个64KB内。在PC系统开机复位时，CPU进入实模式，并将CS寄存器设置成0xF000，将它的shadow register的Base值初始化设置为0xFFFF0000，EIP寄存器初始化设置为0x0000FFF0。所以机器执行的第一条指令的物理地址是0xFFFFFFF0。80386的BIOS代码也要和以前8086的BIOS代码兼容，故地址0xFFFFFFF0处的指令还是一条长跳转指令`jmp F000:E05B`。注意，这个长跳转指令会触发更新CS寄存器和它的shadow register，即执行`jmp F000 : E05B`后，CS将被更新成0xF000。表面上看CS其实没有变化，但CS的shadow register被更新为另外一个值了，它的Base域被更新成0x000F0000，此时形成的物理地址为Base+EIP=0x000FE05B，这就是CPU执行的第二条指令的地址。此时这条指令的地址已经是1M以内了，且此地址不再位于BIOS ROM中，而是位于RAM空间中。由于Intel设计了一种映射机制，将内存高端的BIOS ROM映射到1MB以内的RAM空间里，并且可以使这一段被映射的RAM空间具有与ROM类似的只读属性。所以PC机启动时将开启这种映射机制，让4GB地址空间的最高一个64KB的内容等同于1MB地址空间的最高一个64K的内容，从而使得执行了长跳转指令后，其实是回到了早期的8086 CPU初始化控制流，保证了向下兼容。

回顾知识：
    BIOS (Basic Input Output System，即基本输入/输出系统，其本质是一个固化在主板Flash/CMOS上的软件)和位于软盘/硬盘引导扇区中的OS BootLoader（在ucore中的bootasm.S和bootmain.c）一起组成。


  ----->BIOS的代码固化在EPROM中。在基于Intel的8086 CPU的PC机中的EPROM被编址在1ＭB内存地址空间的最高64KB中。
    因此cs:f000  eip:fff0 在EPROM中
1.   CPU从物理地址0xFFFFFFF0--->shadow register 为0xffff0000（由初始化的CS：EIP确定，此时CS和IP的值分别是
    0xF000和0xFFF0)）开始执行。在0xFFFFFFF0这里只是存放了一条跳转指令，通过跳转0xfffffff0指令跳到BIOS例行程序起始点。
      0xfffffff0:  ljmp   $0xf000,$0xe05b(占3个字节单元)

2.  在0xFFFFFFF0这里只是存放了一条跳转指令，通过跳转指令跳到BIOS例行程序起始点。
    BIOS做完计算机硬件自检和初始化后，会选择一个启动设备（例如软盘、硬盘、光盘等），并且读取该设备的第一扇区(即主引导扇区或启动扇区)到内存一个特定的地址0x7c00处，然后CPU控制权会转移到那个地址继续执行。至此BIOS的初始化工作做完了，进一步的工作交给了ucore的bootloader。
--------------------
1. 系统加电，BIOS初始化硬件
    -->打开A20进入<这是由于在还是20位地址时，BIOS固件放在1MB的最上方，
1. boot/bootasm.S  | bootasm.asm（修改了名字，以便于彩色显示）
 a. 开启A20   16位地址线 实现 20位地址访问  芯片版本兼容
    通过写 键盘控制器8042  的 64h端口 与 60h端口。
    

 ab.物理内存探测  通过 BIOS 中断获取内存布局 =====比lab1多的部分===========

 b. 加载GDT全局描述符 lgdt gdtdesc
 c. 使能和进入保护模式 置位 cr0寄存器的 PE位 (内存分段访问) PE+PG（分页机制）
    movl %cr0, %eax 
    orl $CR0_PE_ON, %eax  或操作，置位 PE位 
    movl %eax, %cr0
 d. 调用载入系统的函数 call bootmain  # 转而调用 bootmain.c 

2. boot/bootmain.c -> bootmain 函数
 a. 调用readseg函数从ELFHDR处读取8个扇区大小的 os 数据。
 b. 将输入读入 到 内存中以 进程(程序)块 proghdr 的方式存储
 c. 跳到ucore操作系统在内存中的入口位置（kern/init.c中的kern_init函数的起始地址）

3. kern/init.c
 a. 初始化终端 cons_init(); init the console   kernel/driver/consore.c
     显示器初始化       cga_init();    
     串口初始化         serial_init(); 
     keyboard键盘初始化 kbd_init();

 b. 打印内核信息 & 欢迎信息 
    print_kerninfo();          //  内核信息  kernel/debug/kdebug.c
    cprintf("%s\n\n", message);//　欢迎信息 const char *message = “qwert”

 c. 显示堆栈中的多层函数调用关系 切换到保护模式，启用分段机制
    grade_backtrace();

 d. 初始化物理内存管理
    pmm_init();        // init physical memory management   kernel/mm/ppm.c
    --->gdt_init();    // 初始化默认的全局描述符表
    ---> page_init();// 内存管理等函数  ===============比lab1多的部分=================

 e. 初始化中断控制器，
    pic_init();        // 初始化 8259A 中断控制器   kernel/driver/picirq.c

 f. 设置中断描述符表
    idt_init();        // kernel/trap/trap.c 
    // __vectors[] 来对应中断描述符表中的256个中断符  tools/vector.c中

 g. 初始化时钟中断，使能整个系统的中断机制  8253定时器 
    clock_init();      // 10ms 时钟中断(1s中断100次)   kernel/driver/clock.c
    ----> pic_enable(IRQ_TIMER);// 使能定时器中断 

 h. 使能整个系统的中断机制 enable irq interrupt
    intr_enable();     // kernel/driver/intr.c
    // sti();          // set interrupt // x86.h

 i. lab1_switch_test();// 用户切换函数 会 触发中断用户切换中断

4. kernel/trap/trap.c 
   trap中断(陷阱)处理函数
    trap() ---> trap_dispatch()   // kernel/trap/trap.c 

    a. 10ms 时钟中断处理 case IRQ_TIMER：
       if((ticks++)%100==0) print_ticks();//向终端打印时间信息（1s打印一次）

    b. 串口1 中断    case IRQ_COM1: 
       获取串口字符后打印

    c. 键盘中断      case IRQ_KBD: 
       获取键盘字符后打印

    d. 用户切换中断
-----------------------------------------------------------------
> kern_init ->
    grade_backtrace ->
        grade_backtrace0(0, (int)kern_init, 0xffff0000) ->
                grade_backtrace1(0, 0xffff0000) ->
                    grade_backtrace2(0, (int)&0, 0xffff0000, (int)&(0xffff0000)) ->
                        mon_backtrace(0, NULL, NULL) ->
                            print_stackframe ->
-----------------------------------------------------------------
    bash
    |-- boot
    | |-- asm.h
    | |-- bootasm.S 增加了对计算机系统中物理内存布局的探测功能；
    | \`-- bootmain.c 
    |-- kern
    | |-- init
    | | |-- entry.S 根据临时段表重新暂时建立好新的段空间，为进行分页做好准备。
    | | \`-- init.c
    | |-- mm
    | | |-- default\_pmm.c
    | | |-- default\_pmm.h 提供基本的基于链表方法的物理内存管理（分配单位为页，即4096字节）；
    | | |-- memlayout.h 
    | | |-- mmu.h    
    | | |-- pmm.c   pmm.c包含了对此物理内存管理类框架的访问，以及与建立、修改、访问页表相关的各种函数实现。
    | | \`-- pmm.h     pmm.h定义物理内存管理类框架struct pmm_manager，基于此通用框架可以实现不同的物理内存管理策略和算法(default_pmm.[ch]
实现了一个基于此框架的简单物理内存管理策略)；
    | |-- sync
    | | \`-- sync.h    为确保内存管理修改相关数据时不被中断打断，提供两个功能，一个是保存eflag寄存器中的中断屏蔽位信息并屏蔽中断的功能，另一个是根据保存的中断屏蔽位信息来使能中断的功能；（可不用细看）
    | \`-- trap
    | |-- trap.c
    | |-- trapentry.S
    | |-- trap.h
    | \`-- vectors.S
    |-- libs
    | |-- atomic.h   定义了对一个变量进行读写的原子操作，确保相关操作不被中断打断。（可不用细看
    | |-- list.h   定义了通用双向链表结构以及相关的查找、插入等基本操作，这是建立基于链表方法的物理内存管理（以及其他内核功能）的基础。其他有类似双向链表需求的内核功能模块可直接使用list.h中定义的函数
    \`-- tools
    |-- kernel.ld   ld形成执行文件的地址所用到的链接脚本。修改了ucore的起始入口和代码段的起始地址。相关细节可参看附录C。
-------------------------------------------------------------

    lab1的整体目录结构如下所示：
>>> tree
  .
  ├── bin  // =======编译后生成======================================
  │   ├── bootblock  // 是引导区
  │   ├── kernel     // 是操作系统内核
  │   ├── sign       // 用于生成一个符合规范的硬盘主引导扇区
  │   └── ucore.img　// ucore.img 通过dd指令，将上面我们生成的　bootblock　和　kernel 的ELF文件拷贝到ucore.img
  ├── boot　// =======bootloader 代码=================================
  │   ├── asm.h      // 是bootasm.S汇编文件所需要的头文件, 是一些与X86保护模式的段访问方式相关的宏定义.
  │   ├── bootasm.S　// 0. 定义了最先执行的函数start，部分初始化，从实模式切换到保护模式，调用bootmain.c中的bootmain函数
  │   └── bootmain.c // 1. 实现了bootmain函数, 通过屏幕、串口和并口显示字符串,加载ucore操作系统到内存，然后跳转到ucore的入口处执行.
  |                  // 生成 bootblock.out 
  |                  // 由 sign.c 在最后添加 0x55AA之后生成 规范的 512字节的
  ├── kern  // =======ucore系统部分===================================
  │   ├── debug// 内核调试部分 ==================================================
  │   │   ├── assert.h   // 保证宏 assert宏，在发现错误后调用 内核监视器kernel monitor
  │   │   ├── kdebug.c　 // 提供源码和二进制对应关系的查询功能，用于显示调用栈关系。
  │   │   ├── kdebug.h   // 其中补全print_stackframe函数是需要完成的练习。其他实现部分不必深究。
  │   │   ├── kmonitor.c // 实现提供动态分析命令的kernel monitor，便于在ucore出现bug或问题后，
  │   │   ├── kmonitor.h // 能够进入kernel monitor中，查看当前调用关系。
  │   │   ├── panic.c    // 内核错误(Kernel panic)是指操作系统在监测到内部的致命错误,
  │   │   └── stab.h
  │   ├── driver　//驱动==========================================================
  │   │   ├── clock.c    // 实现了对时钟控制器8253的初始化操作 系统时钟 
  │   │   ├── clock.h   
  │   │   ├── console.c  // 实现了对串口和键盘的中断方式的处理操作 串口命令行终端
  │   │   ├── console.h
  │   │   ├── intr.c     // 实现了通过设置CPU的eflags来屏蔽和使能中断的函数
  │   │   ├── intr.h
  │   │   ├── kbdreg.h   // 
  │   │   ├── picirq.c   // 实现了对中断控制器8259A的初始化和使能操作   
  │   │   └── picirq.h
  │   ├── init　// 系统初始化======================================================
  │   │   └── init.c       // ucore操作系统的初始化启动代码
  │   ├── libs
  │   │   ├── readline.c
  │   │   └── stdio.c
  │   ├── mm　// 内存管理 Memory management========================================
  │   │   ├── memlayout.h  // 操作系统有关段管理（段描述符编号、段号等）的一些宏定义
  │   │   ├── mmu.h        // 内存管理单元硬件 Memory Management Unit 将线性地址映射为物理地址,包括EFLAGS寄存器等段定义
  │   │   ├── pmm.c　　　　 // 设定了ucore操作系统在段机制中要用到的全局变量
  │   │   └── pmm.h        // 任务状态段ts，全局描述符表 gdt[],加载gdt的函数lgdt，　初始化函数gdt_init
  │   └── trap　// 陷阱trap 异常exception 中断interrupt 中断处理部分=================
  │       ├── trap.c       // 紧接着第二步初步处理后，继续完成具体的各种中断处理操作；
  │       ├── trapentry.S  // 紧接着第一步初步处理后，进一步完成第二步初步处理；
  |       |                // 并且有恢复中断上下文的处理，即中断处理完毕后的返回准备工作；
  │       ├── trap.h       // 紧接着第二步初步处理后，继续完成具体的各种中断处理操作；
  │       └── vectors.S    // 包括256个中断服务例程的入口地址和第一步初步处理实现。
  |                        // 此文件是由tools/vector.c在编译ucore期间动态生成的
  ├── libs　// 公共库部分===========================================================
  │   ├── defs.h           // 包含一些无符号整型的缩写定义
  │   ├── elf.h
  │   ├── error.h
  │   ├── printfmt.c
  │   ├── stdarg.h　　　　　// argument 参数
  │   ├── stdio.h          // 标志输入输出 io
  │   ├── string.c
  │   ├── string.h
  │   └── x86.h            // 一些用GNU C嵌入式汇编实现的C函数
  ├── Makefile             // 指导make完成整个软件项目的编译，清除等工作。
  └── tools　// 工具部分============================================================
      ├── function.mk      // mk模块　指导make完成整个软件项目的编译，清除等工作。
      ├── gdbinit　　　　   // gnu debugger 调试
      ├── grade.sh
      ├── kernel.ld
      ├── sign.c           // 一个C语言小程序，是辅助工具，用于生成一个符合规范的硬盘主引导扇区。
      |                    // 规范的硬盘主引导扇区大小为512字节，结束符为0x55AA
      |                    // obj/bootblock.out( <= 500 )  +  0x55AA -> bootblock(512字节)
      └── vector.c         // 生成vectors.S　中断服务例程的入口地址和第一步初步处理实现

-------------------------------------------------------------