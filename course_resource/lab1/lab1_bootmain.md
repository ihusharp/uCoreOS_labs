-----------bootloader如何读取硬盘扇区的？
1. 等待硬盘空闲。waitdisk的函数实现只有一行：while ((inb(0x1F7) & 0xC0) != 0x40)，意思是不断查询读0x1F7寄存器的最高两位，直到最高位为0、次高位为1（这个状态应该意味着磁盘空闲）才返回。
2. 硬盘空闲后，发出读取扇区的命令。对应的命令字为0x20，放在0x1F7寄存器中；读取的扇区数为1，放在0x1F2寄存器中；读取的扇区起始编号共28位，分成4部分依次放在0x1F3~0x1F6寄存器中。
3. 发出命令后，再次等待硬盘空闲。
4. 硬盘再次空闲后，开始从0x1F0寄存器中读数据。注意insl的作用是"That function will read cnt dwords from the input port specified by port into the supplied output array addr."，是以dword即4字节为单位的，因此这里SECTIZE需要除以4.
   

-----------bootloader是如何加载ELF格式的OS？
    
通过bootmain函数从硬盘中 加载elf格式的 操作系统os 到内存中使用程序块方式存储

 1. 将一些OS的ELF文件从硬盘中读到内存的ELFHDR里面 格式在elf.h中定义
 2. 在加载操作开始之前我们需要对ELFHDR进行判断，观察是否是一个合法的ELF头
 3. 通过循环读取每个段，并且将每个段读入相应的虚存p_va 程序块中
 	---->//一个段含有8个扇区   循环将每个扇区读入虚拟内存中
 4. 最后调用ELF header表头中的内核入口地址, 实现 内核链接地址 转化为 加载地址，无返回值。

 sector 扇区
访问第一个硬盘的扇区可设置IO地址寄存器0x1f0-0x1f7实现

static void waitdisk(void) 判断是否为不忙碌状态
/* readsect - read a single sector at @secno into @dst */
static void readsect(void *dst, uint32_t secno)
static void readseg(uintptr_t va, uint32_t count, uint32_t offset)
void bootmain(void)
-------------最后总结*:
bootloader都干了什么
    关闭中断，
    A20 使能
    全局描述符表初始化
    保护模式启动
    设置段寄存器（长跳转更新CS，根据设置好的段选择子更新其他段寄存器）
    设置堆栈，esp 0x700 ebp 0  ?这个还母鸡
    进入bootmain后读取内核映像到内存，检查是否合法，并启动操作系统，控制权交给它
内存 0xa0000 到 0x100000 属于设备区，而 xv6 内核则是放在 0x100000 处。
为了方便，ucore假设
内核放在引导磁盘中从扇区1开始的连续空间中。
而通常内核就放在普通的文件系统中，而且可能不是连续的。也有可能内核是通过网络加载的。
ucore假设在开机后，引导加载器运行前，唯一发生的事即 BIOS 加载引导扇区。

但实际上 BIOS 会做相当多的初始化工作来确保现代计算机中结构复杂的硬件能像传统标准中的 PC 一样工作。

-----------------------

第6位：为1=LBA模式；0 = CHS模式 第7位和第5位必须为1
-
IO地址	功能
0x1f0	读数据，当0x1f7不为忙状态时，可以读。
0x1f2	要读写的扇区数，每次读写前，你需要表明你要读写几个扇区。最小是1个扇区
0x1f3	如果是LBA模式，就是LBA参数的0-7位
0x1f4	如果是LBA模式，就是LBA参数的8-15位
0x1f5	如果是LBA模式，就是LBA参数的16-23位
0x1f6	第0~3位：如果是LBA模式就是24-27位 第4位：为0主盘；为1从盘

0x1f7	状态和命令寄存器。操作时先给命令，再读取，如果不是忙状态就从0x1f0端口读数据
------------------------------------------
寄存器 			端口 	作用
data寄存器 		0x1F0 	已经读取或写入的数据，大小为两个字节（16位数据)
每次读取1个word,反复循环，直到读完所有数据
features寄存器 	0x1F1 	读取时的错误信息
写入时的额外参数
sector count寄存器 	0x1F2 	指定读取或写入的扇区数
LBA low寄存器 	0x1F3 	lba地址的低8位
LBA mid寄存器 	0x1F4 	lba地址的中8位
LBA high寄存器 	0x1F5 	lba地址的高8位
device寄存器 	0x1F6 	lba地址的前4位（占用device寄存器的低4位）
主盘值为0（占用device寄存器的第5位）
第6位值为1
LBA模式为1，CHS模式为0（占用device寄存器的第7位）
第8位值为1
command寄存器 	0x1F7 	读取，写入的命令，返回磁盘状态1 
读取扇区:0x20 
写入扇区:0x30
磁盘识别:0xEC
----------------------------------------------------
bootloader把内核从硬盘导入到0x100000处，至于为什么不放在0x0开始的地方是因为从640kb到0x100000开始的地方是被用于IO device的映射，所以为了保持内核代码的连续性，就从1MB的内存区域开始存放。在xv6里面，内核对应的虚拟地址实际上是从0x800000处开始的，那么为什么不干脆把内核导入到物理地址的0x800000处呢？这个原因主要是有的小型PC是没有这么高的地址的，所以放在0x100000处显然是个更好的选择。

#define SECTSIZE        512
#define ELFHDR          ((struct elfhdr *)0x10000)      // scratch space
-
static void
waitdisk(void) { //判断是否为不忙碌状态
//如果0x1F7的最高2位是01，跳出循环
    while ((inb(0x1F7) & 0xC0) != 0x40)
// 0x1F7 0号硬盘　读时状态寄存器
// 0xC0 = 0x11000000 最高两位为1
// 0x40 = 0x01000000 01表示空闲
// 检查0x1F7的最高两位，如果是01，那么证明磁盘准备就绪，跳出循环，否则继续等待。

inb 从I/O端口读取一个字节(BYTE, HALF-WORD)

#	&与计算
        /* do nothing */;
}

其中0x1f7地址是状态和命令寄存器的地址，具体的状态细节这里不深究了，总之waitdisk函数就是一直通过获取0x1f7处的地址的状态值判断是否为不忙碌状态

//读取一整块扇区
// readsect - read a single sector at @secno into @dst 
static void
readsect(void *dst, uint32_t secno) {
    // wait for disk to be ready
    waitdisk();				//判断是否为不忙碌状态
 void outb(word port, byte value); 

# 向port写入value
# LBA28方式使用28位来描述一个扇区地址，最大支持128GB的硬磁盘容量。   
    outb(0x1F2, 1);        	//0x1f2存放要读写的扇区数量
    outb(0x1F3, secno & 0xFF);  //0x1f3 存放要读取的扇区编号
    outb(0x1F4, (secno >> 8)&0xFF);//用来存放读写柱面的低8位字节 
    outb(0x1F5, (secno >> 16)&0xFF);//LBA24-27
          // 用来存放要读/写的磁盘号及磁头号
    outb(0x1F6, ((secno >> 24) & 0xF) | 0xE0);
      // 上面四条指令联合制定了扇区号
      // 在这4个字节联合构成的32位参数中
      //   29-31位强制设为1
      //   28位(=0)表示访问"Disk 0"
      //   0-27位是28位的偏移量
    outb(0x1F7, 0x20);       // 0x20读取扇区
https://www.cnblogs.com/mlzrq/p/10223060.html各个端口作用 
其中0x1f2是规定要读写的扇区数
0x1f3 1f4 1f5以及1f6的0到3位，这些位组合起来表示LBA28位参数（secno），0x1f6的第4位，也就是这四个字节的32位的第28位设置为0（|0xE0），0表示主盘
然后向0x1f7发送命令0x20，表示读取扇区  
*
    // wait for disk to be ready
    waitdisk();
*
    // read a sector
    insl(0x1F0, dst, SECTSIZE / 4); //获取数据
    // 读取到dst位置，
	注意insl命令，这个命令定义在x86.h中，其实就是从0x1f0读取SECTSIZE/4个双字到dst位置的汇编实现，注意这里是以双字为单位，即4个字节，所以才除以4，而且注意这里很多命令最后的“l”都对应了实际命令中的“d”
}
-----------------------------------------------------
#define SECTSIZE        512
#define ELFHDR          ((struct elfhdr *)0x10000)      // scratch space*
 * 从0号硬盘上读入os文件
 * 第一个参数是一个虚拟地址va (virtual address)，起始地址
 * 第二个是count（我们所要读取的数据的大小 512*8)*
 *     SECTSIZE的定义我们通过追踪可以看到是512，即一个扇区的大小
 * 第三个是offset（偏移量）

    void bootmain(void) {
    	# 首先从硬盘中将bin/kernel文件的第一页内容加载到内存地址为0x10000的位置，目的是读取kernel文件的ELF Header信息。
        // 首先读取ELF的头部  从磁盘上读取第一页
        readseg((uintptr_t)ELFHDR, SECTSIZE * 8, 0);
        512*8表示512字节 8bit
-
        // 通过储存在头部的magic判断是否是合法的ELF文件
        if (ELFHDR->e_magic != ELF_MAGIC) {
            goto bad;	//不是就out
        }

        struct proghdr *ph, *eph;
        接下来我们需要开始从磁盘中加载OS，首先定义了两个程序头表段，
        ph，eph，其中ph表示ELF段表首地址，eph表示ELF段表末地址：
        
        // ELF头部有描述ELF文件应加载到内存什么位置的描述表，
        // 先将描述表的头地址存在ph
        //    entry 程序入口的虚拟地址
	    //    phoff 表示程序头表的地址偏移
        ph = (struct proghdr *)((uintptr_t)ELFHDR + ELFHDR->e_phoff);
        eph = ph + ELFHDR->e_phnum;//num段表数
        
        // 按照描述表将ELF文件中数据载入内存
        //将每个段读入，最终读入所有内核映像
        for (; ph < eph; ph ++) {
            readseg(ph->p_va & 0xFFFFFF, ph->p_memsz, ph->p_offset);
         	//  p_va，一个对应当前段的虚拟地址
        }
        // ELF文件0x1000位置后面的0xd1ec比特被载入内存0x00100000
        // ELF文件0xf000位置后面的0x1d20比特被载入内存0x0010e000
        
	    // 4. 最后调用ELF header表头中的内核入口地址, 实现 内核链接地址 转化为 加载地址，无返回值
        ((void (*)(void))(ELFHDR->e_entry & 0xFFFFFF))();

    bad:
        outw(0x8A00, 0x8A00);
        outw(0x8A00, 0x8E00);
        while (1);
    }

------------------------------------------------
//读每一个段的内容，一个段含有8个扇区
//循环将每个扇区读入虚拟内存中
 * readseg - read @count bytes at @offset from kernel into virtual address @va,
 * might copy more than asked.
 * 从0号硬盘上读入os文件
 * 第一个参数是一个虚拟地址va (virtual address)，起始地址
 * 第二个是count（我们所要读取的数据的大小 512*8）*，
 *            SECTSIZE的定义我们通过追踪可以看到是512，即一个扇区的大小
 * 第三个是offset（偏移量）
static void
    readseg(uintptr_t va, uint32_t count, uint32_t offset) {
        uintptr_t end_va = va + count;  
        // 转至分区边界
        //　SECTSIZE＝512扇区单位长度　起始地址减去偏移　块首地址
        va -= offset % SECTSIZE; // #define SECTSIZE        512

        uint32_t secno = (offset / SECTSIZE) + 1; 
        // 存储我们需要读取的磁盘的位置
        // 加1因为0扇区被引导占用
        // ELF文件从1扇区开始

// 通过一个for循环一次从磁盘中读取一个整块
    for (; va < end_va; va += SECTSIZE, secno ++) {
        // 继续对虚存va和secno进行自加操作，直到读完所需读的东西为止。
        readsect((void *)va, secno);// 磁盘中读取一个整块 存到相应的虚存va中*
        }
    }
-----------------------------------------------------------------
ELF(Executable and linking format)文件格式是Linux系统下的一种常用目标文件(object file)格式，有三种主要类型:
	1. 用于执行的可执行文件(executable file)，用于提供程序的进程映像，加载的内存执行。 这也是本实验的OS文件类型。
    2. 用于连接的可重定位文件(relocatable file)，可与其它目标文件一起创建可执行文件和共享目标文件。
    3. 共享目标文件(shared object file),连接器可将它与其它可重定位文件和共享目标文件连接成

ELF的文件头包含整个执行文件的控制结构，其定义在elf.h中：
/* file header */
struct elfhdr {
    uint32_t e_magic;     // 判断读出来的ELF格式的文件是否为正确的格式
    uint8_t e_elf[12];
    uint16_t e_type;      // 1=可重定位，2=可执行，3=共享对象，4=核心映像
    uint16_t e_machine;   // 3=x86，4=68K等.
    uint32_t e_version;   // 文件版本，总是1
    uint32_t e_entry;     // 程序入口所对应的虚拟地址。
    uint32_t e_phoff;     // 程序头表的位置偏移
    uint32_t e_shoff;     // 区段标题或0的文件位置
    uint32_t e_flags;     // 特定于体系结构的标志，通常为0
    uint16_t e_ehsize;    // 这个elf头的大小
    uint16_t e_phentsize; // 程序头中条目的大小
    uint16_t e_phnum;     // 程序头表中的入口数目
    uint16_t e_shentsize; // 节标题中条目的大小
    uint16_t e_shnum;     // 节标题中的条目数或0
    uint16_t e_shstrndx;  // 包含节名称字符串的节号。
};



    magic，这个是用于检验是否是一个合法的elf文件的
    entry 程序入口的虚拟地址
    phoff 表示程序头表的地址偏移
    phnum 表示程序头表的段数目

program header描述与程序执行直接相关的目标文件结构信息，用来在文件中定位各个段的映像，同时包含其他一些用来为程序创建进程映像所必需的信息。
Program header描述的是一个段在文件中的位置、大小以及它被放进内存后所在的位置和大小。

头表中的段，
    struct proghdr {
      uint type;   // 段类型
      uint offset;  // 段相对文件头的偏移值
      uint va;     // 段的第一个字节将被放到内存中的虚拟地址
      uint pa;
      uint filesz;
      uint memsz;  // 段在内存映像中占用的字节数
      uint flags;
      uint align;
    };

    在这里我们需要了解一些参数，
    p_va，一个对应当前段的虚拟地址；p_memsz，当前段的内存大小；
    p_offset，段相对于文件头的偏移。
了解了程序在磁盘和内存中分别的存储方式之后我们就需要开始从内存中读取数据加载到内存中来。由于上问的操作，我们将一些OS的ELF文件读到ELFHDR里面，所以在加载操作开始之前我们需要对ELFHDR进行判断，观察是否是一个合法的ELF头：
