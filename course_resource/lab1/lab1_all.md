调用顺序：
  1. boot/bootasm.S  | bootasm.asm
     a. 开启A20   16位地址线 实现 20位地址访问  芯片版本兼容
        通过写 键盘控制器8042  的 64h端口 与 60h端口。
        
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
           
---------------------------------------------------------------

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
