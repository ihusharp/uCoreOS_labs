
练习2：实现寻找虚拟地址对应的页表项（需要编程）

通过设置页表和对应的页表项，可建立虚拟内存地址和物理内存地址的对应关系。其中的get_pte函数是设置页表项环节中的一个重要步骤。此函数找到一个虚地址对应的二级页表项的内核虚地址，如果此二级页表项不存在，则分配一个包含此项的二级页表。本练习需要补全get_pte函数in

（1）三种地址
x86体系结构将内存地址分成三种：逻辑地址（也称虚地址）、线性地址和物理地址。
-逻辑地址即是程序指令中使用的地址。
-物理地址是实际访问内存的地址。
-逻辑地址通过段式管理的地址映射可以得到线性地址，线性地址通过页式管理的地址映射得到物理地址。get pte函数是给出了线性地址，即linear address。

---------------------------------------------------------
// A linear address 'la' has a three-part structure as follows:
//
// +--------10------+-------10-------+---------12----------+
// | Page Directory |   Page Table   | Offset within Page  |
// |      Index     |     Index      |                     |
// +----------------+----------------+---------------------+
//  \--- PDX(la) --/ \--- PTX(la) --/ \---- PGOFF(la) ----/
//  \----------- PPN(la) -----------/
//
// The PDX, PTX, PGOFF, and PPN macros decompose linear addresses as shown.
// To construct a linear address la from PDX(la), PTX(la), and PGOFF(la),
// use PGADDR(PDX(la), PTX(la), PGOFF(la)).

// page directory index
#define PDX(la) ((((uintptr_t)(la)) >> PDXSHIFT) & 0x3FF)

// page table index
#define PTX(la) ((((uintptr_t)(la)) >> PTXSHIFT) & 0x3FF)
-------------------------------------------------------------
 Some Useful MACROs and DEFINEs, you can use them in below implementation.
 * MACROs or Functions:
 *   PDX(la) = the index of page directory entry of VIRTUAL ADDRESS la.
 *   KADDR(pa) : takes a physical address and returns the corresponding kernel virtual address.
 	返回pa对应的虚拟地址（线性地址
 *   set_page_ref(page,1) : means the page be referenced by one time 引用一次
 *   page2pa(page): get the physical address of memory which this (struct Page *) page  manages 
		得到page管理的那一页的物理地址
 *   struct Page * alloc_page() : allocation a page
 *   memset(void *s, char c, size_t n) : sets the first n bytes of the memory area pointed by s
 *                 to the specified value c.
 * DEFINEs:*
 *   PTE_P           0x001                   // page table/directory entry flags bit : Present
 *   PTE_W           0x002                   // page table/directory entry flags bit : Writeable
 *   PTE_U           0x004                   // page table/directory entry flags bit : User can access
-----------------------------------------------------
通过查二级页表 打开对应的物理映射——->即二级页表表项作为指针来查地址
pte_t *
get_pte(pde_t *pgdir, uintptr_t la, bool create)
------>*
//get_pte - get pte and return the kernel virtual address of this pte for la
//        - if the PT contians this pte didn't exist, alloc a page for PT
// parameter:
//  pgdir:  the kernel virtual base address of PDT
//  la:     the linear address need to map
//  create: a logical value to decide if alloc a page for PT

// return vaule: the kernel virtual address of this pte
-----------
	pde_t全称为 page directory
	entry，也就是一级页表的表项（注意：pgdir实际不是表项，而是一级页表本身。实际上应该新定义一个类型pgd_t来表示一级页表本身）。
	pte t 全称为 page table entry，表示二级页表的表项。
	uintptrt表示为线性地址，由于段式管理只做直接映射，所以它也是逻辑地址
	
	pgdir给出页表起始地址。通过查找这个页表，我们需要给出二级页表中对应项的地址。
	虽然目前我们只有boot_pgdir一个页表，但是引入进程的概念之后每个进程都会有自己的页表。
	
	有可能根本就没有对应的二级页表的情况，所以二级页表不必要一开始就分配，而是等到需要的时候再添加对应的二级页表。如果在查找二级页表项时，发现对应的二级页表不存在，则需要根据create参数的值来处理是否创建新的二级页表。如果create参数为0，则get_pte返回NULL；如果create参数不为0，则get_pte需要申请一个新的物理页（通过alloc_page来实现，可在mm/pmm.h中找到它的定义），再在一级页表中添加页目录项指向表示二级页表的新物理页。注意，新申请的页必须全部设定为零，因为这个页所代表的虚拟地址都没有被映射。

-------------------------------------------------
一级页表的起始物理地址存放在 cr3
寄存器中，这个地址必须是一个页对齐的地址，也就是低 12 位必须为
0。目前，ucore 用boot_cr3（mm/pmm.c）记录这个值。

------------------------------------------------
我们可以看到，在这种模式下，逻辑地址先通过段机制转化成线性地址， 然后通过两种页表(页目录和页表)来实现线性地址到物理地址的转换。 有一点需要注意，在页目录和页表中存放的地址都是物理地址。

-----------------------------------------------
/* All physical memory mapped at this address */*
#define KERNBASE            0xC0000000
#define KMEMSIZE            0x38000000                  // the maximum amount of physical memory

#define KERNTOP             (KERNBASE + KMEMSIZE)

查看 bootloader的实现代码 bootmain::bootmain.c

readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);

这里的ph->p_va=0xC0XXXXXX，就是ld工具根据kernel.ld设置的链接地址，且链接地址等于虚地址。
考虑到ph->p_va & 0xFFFFFF == 0x0XXXXXX，所以bootloader加载ucore
kernel的加载地址是0x0XXXXXX, 这实际上是ucore内核所在的物理地址。
--简言之：
OS的链接地址（link addr） 在tools/kernel.ld中设置好了，是一个虚地址（virtual
addr）；
而ucore kernel的加载地址（load addr）在boot
loader中的bootmain函数中指定，是一个物理地址。

小结一下，
ucore内核的链接地址==ucore内核的虚拟地址；
bootloader加载ucore内核用到的加载地址==ucore内核的物理地址

--------------------------------------------------
在基于ELF执行文件格式的代码中，存在一些对代码和数据的表述，基本概念如下：
    BSS段（bss
    segment）：指用来存放程序中未初始化的全局变量的内存区域。BSS是英文Block
    Started by Symbol的简称。BSS段属于静态内存分配。
    数据段（data
    segment）：指用来存放程序中已初始化的全局变量的一块内存区域。数据段属于静态内存分配。
    代码段（code segment/text
    segment）：指用来存放程序执行代码的一块内存区域。这部分区域的大小在程序运行前就已经确定，并且内存区域通常属于只读,
    某些架构也允许代码段为可写，即允许修改程序。在代码段中，也有可能包含一些只读的常数变量，例如字符串常量等。
--------------------
在lab2/kern/init/init.c的kern_init函数中，声明了外部全局变量：

    extern char edata[], end[];

“edata”表示数据段的结束地址，“.bss”表示数据段的结束地址和BSS段的起始地址，而“end”表示BSS段的结束地址。

这样回头看kerne_init中的外部全局变量，可知edata[]和
end[]这些变量是ld根据kernel.ld链接脚本生成的全局变量，表示相应段的起始地址或结束地址等，它们不在任何一个.S、.c或.h文件中定义。
-------------------------------------------------------------
。在二级页表结构中，页目录表占4KB空间，可通过alloc_page函数获得一个空闲物理页作为页目录表（Page Directory Table，PDT）。同理，ucore也通过这种类似方式获得一个页表(Page Table,PT)所需的4KB空间。
一个页目录项(Page Directory Entry，PDE)和一个页表项(Page Table Entry，PTE)占4B
即使是4个页目录项也需要一个完整的页目录表（占4KB）
--------------
整个页目录表和页表所占空间大小取决与二级页表要管理和映射的物理页数。假定当前物理内存0~16MB，每物理页（也称Page Frame）大小为4KB，则有4096个物理页，也就意味这有4个页目录项和4096个页表项需要设置。一个页目录项(Page Directory Entry，PDE)和一个页表项(Page Table Entry，PTE)占4B。即使是4个页目录项也需要一个完整的页目录表（占4KB）。而4096个页表项需要16KB（即4096*4B）的空间，也就是4个物理页，16KB的空间。所以对16MB物理页建立一一映射的16MB虚拟页，需要5个物理页，即20KB的空间来形成二级页表。*~

为把0~KERNSIZE（明确ucore设定实际物理内存不能超过KERNSIZE值，即0x38000000字节，896MB，3670016个物理页）的物理地址一一映射到页目录项和页表项的内容，其大致流程如下~：
    1.先通过alloc_page获得一个空闲物理页，用于页目录表；
    2.调用boot_map_segment函数建立一一映射关系，具体处理过程以页为单位进行设置，即

        virt addr = phy addr + 0xC0000000
---------------
    设一个32bit线性地址la有一个对应的32bit物理地址pa，如果在以la的高10位为索引值的页目录项中的存在位（PTE_P）为0，表示缺少对应的页表空间，则可通过alloc_page获得一个空闲物理页给页表，页表起始物理地址是按4096字节对齐的，这样填写页目录项的内容为
    
        页目录项内容 = (页表起始物理地址 & ~0x0FFF) | PTE_U | PTE_W | PTE_P
    
    进一步对于页表中以线性地址la的中10位为索引值对应页表项的内容为
    
        页表项内容 = (pa & ~0x0FFF) | PTE_P | PTE_W
    
    PTE_U：位3，表示用户态的软件可以读取对应地址的物理内存页内容
    PTE_W：位2，表示物理内存页内容可写
    PTE_P：位1，表示物理内存页存在
----------------
使能分页机制了，
这主要是通过enable_paging函数实现的，
这个函数主要做了两件事：

    通过lcr3指令把页目录表的起始地址存入CR3寄存器中；
    
    通过lcr0指令把cr0中的CR0_PG标志位设置上。

执行完enable_paging函数后，计算机系统进入了分页模式！

但是此时还没有建立完整的段页式机制
我们需要：最终的段映射是简单的段对等映射（virt addr = linear addr）
但之前有一个临时映射，需要进行修改

---------------------------------------------------
#if 0
    pde_t *pdep = &pgdir[PDX(la)];   // (1) find page directory entry
    if ( !(pdep & PTE_P) ) {              // (2) check if entry is not present
                          // (3) check if creating is needed, then alloc page for page table
                          // CAUTION: this page is used for page table, not for common data page
                          // (4) set page reference
        uintptr_t pa = 0; // (5) get linear address of page
                          // (6) clear page content using memset
                          // (7) set page directory entry's permission
    }
    return NULL;          // (8) return page table entry
#endif

-------------------------------------------------
首先，最重要的一点就是要明白页目录和页表中存储的都是物理地址。所以当我们从页目录中获取页表的物理地址后，我们需要使用KADDR()将其转换成虚拟地址。之后就可以用这个虚拟地址直接访问对应的页表了。

第二， *, &, memset() 等操作的都是虚拟地址。注意不要将物理或者线性地址用于这些操作(假设线性地址和虚拟地址不一样)。*

第三，alloc_page()获取的是物理page对应的Page结构体，而不是我们需要的物理page。通过一系列变化(page2pa())，我们可以根据获取的Page结构体得到与之对应的物理page的物理地址，之后我们就能获得它的虚拟地址。

------------------------------------------------------------
回答问题：
1. 请描述页目录项（Page Directory Entry）和页表项（Page Table Entry）中每个组成部分的含义以及对ucore而言的潜在用处。
   

PDE（页目录项）的具体组成如下图所示；描述每一个组成部分的含义如下：
    前20位表示4K对齐的该PDE对应的页表起始位置（物理地址，该物理地址的高20位即PDE中的高20位，低12位为0）；
    第9-11位未被CPU使用，可保留给OS使用；
    接下来的第8位可忽略；
    第7位用于设置Page大小，0表示4KB；
    第6位恒为0；
    第5位用于表示该页是否被使用过；
    第4位设置为1则表示不对该页进行缓存；
    第3位设置是否使用write through缓存写策略；
    第2位表示该页的访问需要的特权级；
    第1位表示是否允许读写；
    第0位为该PDE的存在位；
-------------------------
页表项（PTE）中的每个组成部分的含义，具体组成如下图所示：
    高20位与PDE相似的，用于表示该PTE指向的物理页的物理地址；
    9-11位保留给OS使用；
    7-8位恒为0；
    第6位表示该页是否为dirty，即是否需要在swap out的时候写回外存；
    第5位表示是否被访问；
    3-4位恒为0；
    0-2位分别表示存在位、是否允许读写、访问该页需要的特权级；
-------------------------
可以发现无论是PTE还是TDE，都具有着一些保留的位供操作系统使用，也就是说ucore可以利用这些位来完成一些其他的内存管理相关的算法，比如可以在这些位里保存最近一段时间内该页的被访问的次数（仅能表示0-7次），用于辅助近似地实现虚拟内存管理中的换出策略的LRU之类的算法；也就是说这些保留位有利于OS进行功能的拓展；

---------------------------------------
2. 如果ucore执行过程中访问内存，出现了页访问异常，请问硬件要做哪些事情？

参考网上资料，总结缺页中断发生时的事件顺序如下：

1) 硬件陷入内核，在堆栈中保存程序计数器。大多数机器将当前指令的各种状态信息保存在特殊的CPU寄存器中。
2) 启动一个汇编代码例程保存通用寄存器和其他易失的信息，以免被操作系统破坏。这个例程将操作系统作为一个函数来调用。
3) 当操作系统发现一个缺页中断时，尝试发现需要哪个虚拟页面。通常一个硬件寄存器包含了这一信息，如果没有的话，操作系统必须检索程序计数器，取出这条指令，用软件分析这条指令，看看它在缺页中断时正在做什么。
4) 一旦知道了发生缺页中断的虚拟地址，操作系统检查这个地址是否有效，并检查存取与保护是否一致。如果不一致，向进程发出一个信号或杀掉该进程。如果地址有效且没有保护错误发生，系统则检查是否有空闲页框。如果没有空闲页框，执行页面置换算法寻找一个页面来淘汰。
5) 如果选择的页框“脏”了，安排该页写回磁盘，并发生一次上下文切换，挂起产生缺页中断的进程，让其他进程运行直至磁盘传输结束。无论如何，该页框被标记为忙，以免因为其他原因而被其他进程占用。
6) 一旦页框“干净”后（无论是立刻还是在写回磁盘后），操作系统查找所需页面在磁盘上的地址，通过磁盘操作将其装入。该页面被装入后，产生缺页中断的进程仍然被挂起，并且如果有其他可运行的用户进程，则选择另一个用户进程运行。
7) 当磁盘中断发生时，表明该页已经被装入，页表已经更新可以反映它的位置，页框也被标记为正常状态。
8) 恢复发生缺页中断指令以前的状态，程序计数器重新指向这条指令。
9) 调度引发缺页中断的进程，操作系统返回调用它的汇编语言例程。
10) 该例程恢复寄存器和其他状态信息