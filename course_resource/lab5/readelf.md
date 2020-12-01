**readelf命令，**一般用于查看ELF格式的文件信息，

**ELF Header:** 

魔数 7f ELF

  **Magic:**   7f 45 4c 46 02 01 01 00 00 00 00 00 00 00 00 00 
  Class:                             ELF64
  Data:                              2's complement, little endian
  Version:                           1 (current)
  OS/ABI:                            UNIX - System V
  ABI Version:                       0
  **Type:**                              EXEC (Executable file)
  **Machine:**                           Advanced Micro Devices X86-64
  **Version:**                           0x1
  **Entry point address:**               0x404890
  Start of program headers:          64 (bytes into file)
  Start of section headers:          108288 (bytes into file)
  Flags:                             0x0
  Size of this header:               64 (bytes)
  Size of program headers:           56 (bytes)
  Number of program headers:         9
  Size of section headers:           64 (bytes)
  Number of section headers:         28
  Section header string table index: 27

**Section Headers:**
  [Nr] Name              Type             Address           Offset
       Size              EntSize          Flags  Link  Info  Align
  [ 0]                   NULL             0000000000000000  00000000
       0000000000000000  0000000000000000           0     0     0
  [ 1] .interp           PROGBITS         0000000000400238  00000238
       000000000000001c  0000000000000000   A       0     0     1
  [ 2] .note.ABI-tag     NOTE             0000000000400254  00000254
       0000000000000020  0000000000000000   A       0     0     4
  [ 3] .note.gnu.build-i NOTE             0000000000400274  00000274
       0000000000000024  0000000000000000   A       0     0     4
  [ 4] .gnu.hash         GNU_HASH         0000000000400298  00000298
       0000000000000068  0000000000000000   A       5     0     8
  [ 5] .dynsym           DYNSYM           0000000000400300  00000300
       0000000000000c18  0000000000000018   A       6     1     8
  [ 6] .dynstr           STRTAB           0000000000400f18  00000f18
       0000000000000593  0000000000000000   A       0     0     1
  [ 7] .gnu.version      VERSYM           00000000004014ac  000014ac
       0000000000000102  0000000000000002   A       5     0     2
  [ 8] .gnu.version_r    VERNEED          00000000004015b0  000015b0
       0000000000000090  0000000000000000   A       6     2     8
  [ 9] .rela.dyn         RELA             0000000000401640  00001640
       00000000000000a8  0000000000000018   A       5     0     8
  [10] .rela.plt         RELA             00000000004016e8  000016e8
       0000000000000a80  0000000000000018   A       5    12     8
  [11] .init             PROGBITS         0000000000402168  00002168
       000000000000001a  0000000000000000  AX       0     0     4
  [12] .plt              PROGBITS         0000000000402190  00002190
       0000000000000710  0000000000000010  AX       0     0     16
  [13] .text             PROGBITS         00000000004028a0  000028a0
       000000000000f65a  0000000000000000  AX       0     0     16
  [14] .fini             PROGBITS         0000000000411efc  00011efc
       0000000000000009  0000000000000000  AX       0     0     4
  [15] .rodata           PROGBITS         0000000000411f20  00011f20
       00000000000050fc  0000000000000000   A       0     0     32
  [16] .eh_frame_hdr     PROGBITS         000000000041701c  0001701c
       000000000000072c  0000000000000000   A       0     0     4
  [17] .eh_frame         PROGBITS         0000000000417748  00017748
       00000000000025fc  0000000000000000   A       0     0     8
  [18] .init_array       INIT_ARRAY       0000000000619df0  00019df0
       0000000000000008  0000000000000000  WA       0     0     8
  [19] .fini_array       FINI_ARRAY       0000000000619df8  00019df8
       0000000000000008  0000000000000000  WA       0     0     8
  [20] .jcr              PROGBITS         0000000000619e00  00019e00
       0000000000000008  0000000000000000  WA       0     0     8
  [21] .dynamic          DYNAMIC          0000000000619e08  00019e08
       00000000000001f0  0000000000000010  WA       6     0     8
  [22] .got              PROGBITS         0000000000619ff8  00019ff8
       0000000000000008  0000000000000008  WA       0     0     8
  [23] .got.plt          PROGBITS         000000000061a000  0001a000
       0000000000000398  0000000000000008  WA       0     0     8
  [24] .data             PROGBITS         000000000061a3a0  0001a3a0
       0000000000000254  0000000000000000  WA       0     0     32
  [25] .bss              NOBITS           000000000061a600  0001a5f4
       0000000000000d60  0000000000000000  WA       0     0     32
  [26] .gnu_debuglink    PROGBITS         0000000000000000  0001a5f4
       0000000000000008  0000000000000000           0     0     1
  [27] .shstrtab         STRTAB           0000000000000000  0001a5fc
       00000000000000fe  0000000000000000           0     0     1
Key to Flags:
  W (write), A (alloc), X (execute), M (merge), S (strings), l (large)
  I (info), L (link order), G (group), T (TLS), E (exclude), x (unknown)
  O (extra OS processing required) o (OS specific), p (processor specific)

**Program Headers:**
  Type           Offset             VirtAddr           PhysAddr
                 FileSiz            MemSiz              Flags  Align
  PHDR           0x0000000000000040 0x0000000000400040 0x0000000000400040
                 0x00000000000001f8 0x00000000000001f8  R E    8
  INTERP         0x0000000000000238 0x0000000000400238 0x0000000000400238
                 0x000000000000001c 0x000000000000001c  R      1
      [Requesting program interpreter: /lib64/ld-linux-x86-64.so.2]
  LOAD           0x0000000000000000 0x0000000000400000 0x0000000000400000
                 0x0000000000019d44 0x0000000000019d44  R E    200000
  LOAD           0x0000000000019df0 0x0000000000619df0 0x0000000000619df0
                 0x0000000000000804 0x0000000000001570  RW     200000
  DYNAMIC        0x0000000000019e08 0x0000000000619e08 0x0000000000619e08
                 0x00000000000001f0 0x00000000000001f0  RW     8
  NOTE           0x0000000000000254 0x0000000000400254 0x0000000000400254
                 0x0000000000000044 0x0000000000000044  R      4
  GNU_EH_FRAME   0x000000000001701c 0x000000000041701c 0x000000000041701c
                 0x000000000000072c 0x000000000000072c  R      4
  GNU_STACK      0x0000000000000000 0x0000000000000000 0x0000000000000000
                 0x0000000000000000 0x0000000000000000  RW     10
  GNU_RELRO      0x0000000000019df0 0x0000000000619df0 0x0000000000619df0
                 0x0000000000000210 0x0000000000000210  R      1

 **Section to Segment mapping:**

此处便是将多个属性相同的section合并为同一个segment的介绍

  Segment Sections...
   00     
   01     .interp 
   02     .interp .note.ABI-tag .note.gnu.build-id .gnu.hash .dynsym .dynstr .gnu.version .gnu.version_r .rela.dyn .rela.plt .init .plt .text .fini .rodata .eh_frame_hdr .eh_frame 
   03     .init_array .fini_array .jcr .dynamic .got .got.plt .data .bss 
   04     .dynamic 
   05     .note.ABI-tag .note.gnu.build-id 
   06     .eh_frame_hdr 
   07     
   08     .init_array .fini_array .jcr .dynamic .got 

-----------

![img](20170601180629601-1587112158852.png)

​	ELF segment 和 section 用上图来解释很恰当，其实就是对于elf文件中一部分相同内容的不同描述映射而已，就是上图红框中标出的内容，就好比一个学院的学生，有人喜欢用一班的学生，二班的学生去描述，也有人用女同学，男同学去描述

​	section 称为节，是指在汇编源码中经由关键字section 或segment 修饰、逻辑划分的指令或数据区域，
汇编器会将这两个关键字修饰的区域在目标文件中编译成节，也就是说“**节”最初诞生于目标文件**中。
​	segment 称为段，是链接器根据目标文件中属性相同的多个section 合并后的section 集合，这个集合
称为segment，也就是段，链接器把目标文件链接成可执行文件，因此**段最终诞生于可执行文件**中。我们
平时所说的可执行程序内存空间中的代码段和数据段就是指的segment 。

```
/* file header */
即为头部组成部分，依顺序读下来
struct elfhdr {
    uint32_t e_magic;     // must equal ELF_MAGIC
    uint8_t e_elf[12];
    uint16_t e_type;      // 1=relocatable, 2=executable, 3=shared object, 4=core image
    uint16_t e_machine;   // 3=x86, 4=68K, etc.
    uint32_t e_version;   // file version, always 1
    uint32_t e_entry;     // entry point if executable
    uint32_t e_phoff;     // file position of program header or 0
    uint32_t e_shoff;     // file position of section header or 0
    uint32_t e_flags;     // architecture-specific flags, usually 0
    uint16_t e_ehsize;    // size of this elf header
    uint16_t e_phentsize; // size of an entry in program header
    uint16_t e_phnum;     // number of entries in program header or 0
    uint16_t e_shentsize; // size of an entry in section header
    uint16_t e_shnum;     // number of entries in section header or 0
    uint16_t e_shstrndx;  // section number that contains section name strings
};
```

e_version 占用4 字节，用来表示版本信息。
e_entry 占用4 字节，用来指明操作系统运行该程序时，将控制权转交到的虚拟地址。
e phoff 占用4 字节，用来指明程序头表（ program header table ）在文件内的字节偏移量。如果没有程
序头表，该值为0 。
e_shoff 占用4 字节，用来指明节头表（ section header table ）在文件内的字节偏移量。若没有节头表，
该值为0 。
e_flags 占用4 字节，用来指明与处理器相关的标志，本书用不到那么多的内容，具体取值范围，有兴
趣的同学还是要参考／usr/include/elf.h,
e_ehsize 占用2 字节，用来指明elf header 的宇节大小。
e_phentsize 占用2 字节，用来指明程序头表（ program header table ）中每个条目（ entry ）的字节大小，
即每个用来描述段信息的数据结构的字节大小，该结构是后面要介绍的struct Elf32 Phdr。
e_phnum 占用2 字节，用来指明程序头表中条目的数量。实际上就是段的个数。
e_shentsize 占用2 宇节，用来指明节头表（ section header table ）中每个条目（ en町）的字节大小，即
每个用来描述节信息的数据结构的字节大小。
216
e shnum 占用2 字节，用来指明节头表中条目的数量。实际上就是节的个数。
e_shstrndx 占用2 宇节，用来指明string name table 在节头表中的索引index 。



2.program header

```
/* program section header */ 代码段头部
struct proghdr {
    uint32_t p_type;   // loadable code or data, dynamic linking info,etc.
    uint32_t p_offset; // file offset of segment
    uint32_t p_va;     // virtual address to map segment
    uint32_t p_pa;     // physical address, not used
    uint32_t p_filesz; // size of segment in file
    uint32_t p_memsz;  // size of segment in memory (bigger if contains bss）
    uint32_t p_flags;  // read/write/execute bits
    uint32_t p_align;  // required alignment, invariably hardware page size
};
```

p_type 占用4 字节，用来指明程序中该段的类型。

p_offset 占用4 字节，用来指明本段在文件内的起始偏移字节。
p_vaddr 占用4 字节，用来指明本段在内存中的起始虚拟地址。
p_paddr 占用4 字节，仅用于与物理地址相关的系统中，因为System V 忽略用户程序中所有的物理地
址，所以此项暂且保留，未设定。
p_filesz 占用4 字节，用来指明本段在文件中的大小。
p_memsz 占用4 字节，用来指明本段在内存中的大小。
p_flags 占用4 字节，用来指明与本段相关的标志，此标志取值主要为可写，可执行，可读之类

p_align占用4 字节，用来指明本段在文件和内存中的**对齐方式**。如果值为0 或1 ，则表示不对齐。否
则p_align 应该是2 的事次数。

---------------------

### 关于各段的介绍

## **bss段：**

　　**bss段（bss segment）**通常是指用来存放程序中未初始化的全局变量的一块内存区域。

　　bss是英文Block Started by Symbol的简称。

　　bss段属于静态内存分配。 



## **data段：**

　　**数据段（data segment）**通常是指用来存放程序中已初始化的全局变量的一块内存区域。

　　数据段属于静态内存分配。 



## **text段：**

　　**代码段（code segment/text segment）**通常是指用来存放程序执行代码的一块内存区域。

　　这部分区域的大小在程序运行前就已经确定，并且内存区域**通常属于只读**(某些架构也允许代码段为可写，即允许修改程序)。

　　在代码段中，也有可能包含一些只读的常数变量，例如字符串常量等。 



## **堆（heap）：**

　　堆是用于存放进程运行中被动态分配的内存段，它的大小并不固定，可动态扩张或缩减。

　　当进程调用malloc等函数分配内存时，新分配的内存就被动态添加到堆上（堆被扩张）；

　　当利用free等函数释放内存时，被释放的内存从堆中被剔除（堆被缩减）。



## **栈(stack)**：

　　 栈又称堆栈，是用户存放程序临时创建的局部变量，

　　也就是说我们函数括弧“{}”中定义的变量（但不包括static声明的变量，static意味着在数据段中存放变量）。

　　除此以外，在函数被调用时，其参数也会被压入发起调用的进程栈中，并且待到调用结束后，函数的返回值也会被存放回栈中。

　　由于栈的先进后出特点，所以栈特别方便用来保存/恢复调用现场。

　　从这个意义上讲，我们可以把堆栈看成一个寄存、交换临时数据的内存区。 

------

**一个程序本质上都是由 bss段、data段、text段三个组成的。**

　　这样的概念，不知道最初来源于哪里的规定，但在当前的计算机程序设计中是很重要的一个基本概念。

　　而且在嵌入式系统的设计中也非常重要，牵涉到嵌入式系统运行时的内存大小分配，存储单元占用空间大小的问题。

  在采用段式内存管理的架构中（比如intel的80x86系统），bss段通常是指用来存放程序中未初始化的全局变量的一块内存区域，

　　一般在初始化时bss 段部分将会清零。bss段属于静态内存分配，即程序一开始就将其清零了。

  比如，在C语言之类的程序编译完成之后，已初始化的全局变量保存在.data 段中，未初始化的全局变量保存在.bss 段中。

　　text和data段都在可执行文件中（在嵌入式系统里一般是固化在镜像文件中），由系统从可执行文件中加载；

　　而bss段不在可执行文件中，由系统初始化。

------

## 【例】

两个小程序如下：

程序1:

```
int ar[30000];
void main()
{
    ......
}
```

 


程序2:

```
int ar[300000] = {1, 2, 3, 4, 5, 6 };
void main()
{
    ......
}
```

发现程序2编译之后所得的.exe文件比程序1的要大得多。当下甚为不解，于是手工编译了一下，并使用了/FAs编译选项来查看了一下其各自的.asm，

发现在程序1.asm中ar的定义如下：

_BSS SEGMENT
   ?ar@@3PAHA DD 0493e0H DUP (?)  ; ar
_BSS ENDS
而在程序2.asm中，ar被定义为：

_DATA SEGMENT
   ?ar@@3PAHA DD 01H  ; ar
        DD 02H
        DD 03H
        ORG $+1199988
_DATA ENDS
区别很明显，一个位于.bss段，而另一个位于.data段，两者的区别在于：

全局的未初始化变量存在于.bss段中，具体体现为一个占位符；

全局的已初始化变量存于.data段中；

而函数内的自动变量都在栈上分配空间；

.bss是不占用.exe文件空间的，其内容由操作系统初始化（清零）；

.data却需要占用，其内容由程序初始化。因此造成了上述情况。

bss段（未手动初始化的数据）并不给该段的数据分配空间，只是记录数据所需空间的大小；

bss段的大小从可执行文件中得到 ，然后链接器得到这个大小的内存块，紧跟在数据段后面。

data段（已手动初始化的数据）则为数据分配空间，数据保存在目标文件中；

data段包含经过初始化的全局变量以及它们的值。当这个内存区进入程序的地址空间后全部清零。

 

包含data段和bss段的整个区段此时通常称为数据区。

-------------------

### ELF文件中section与segment的区别

1. ELF中的section主要提供给Linker使用， 而segment提供给Loader用，Linker需要关心.text, .rel.text, .data, .rodata等等，关键是Linker需要做relocation。而Loader只需要知道Read/Write/Execute的属性。a.out格式没有这种区分。（注意现在使用gcc编译出来的a.out文件只是取该名字而已，文件格式是elf的.）

2. 一个executable的ELF文件可以没有section，但必须有segment。ELF文件中间部分是共用的（也就是代码段、数据段等），如shared objects就可以同时拥有Program header table和Section Header Table，这样load完后还可以relocate。


3. 这样设定之后，使得Loader需要做的工作大大减少了，一定程度上提高了程序加载的效率。

-------------------------

### Link addr& Load addr

Link Address是指编译器指定代码和数据所需要放置的内存地址，由链接器配置。Load  Address是指程序被实际加载到内存的位置（由程序加载器ld配置）。一般由可执行文件结构信息和加载器可保证这两个地址相同。Link  Addr和LoadAddr不同会导致：

- 直接跳转位置错误
- 直接内存访问(只读数据区或bss等直接地址访问)错误
- 堆和栈等的使用不受影响，但是可能会覆盖程序、数据区域
  注意：也存在Link地址和Load地址不一样的情况（例如：动态链接库）。