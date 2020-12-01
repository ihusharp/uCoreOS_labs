规定：给 8042 发送命令 0xDF 置 A20 gate 有效，给 8042 送命令 0xDD 置 A20 gate 无效。

8042有4个寄存器
    1个8-bit长的Input buffer；Write-Only；
    1个8-bit长的Output buffer； Read-Only；
    1个8-bit长的Status Register；Read-Only；
    1个8-bit长的Control Register；Read/Write。

有两个端口地址：60h和64h，有关对它们的读写操作描述如下：

    读60h端口，读output buffer
    写60h端口，写input buffer
    读64h端口，读Status Register
    操作Control Register，首先要向64h端口写一个命令（20h为读命令，60h为写命令），然后根据命令从60h端口读出Control Register的数据或者向60h端口写入Control Register的数据（64h端口还可以接受许多其它的命令）。
    
    A20 Gate，出现80286以后，为了保持和8086的兼容，PC机在设计上在第21条地址线（也就是A20）上做了一个开关，当这个开关打开时，这条地址线和其它地址线一样可以使用，当这个开关关闭时，第21条地址线（A20）恒为0，这个开关就叫做A20 Gate，很显然，在实模式下要访问高端内存区，这个开关必须打开，在保护模式下，由于使用32位地址线，如果A20恒等于0，那么系统只能访问奇数兆的内存，即只能访问0--1M、2-3M、4-5M......，这显然是不行的，所以在保护模式下，这个开关也必须打开。


    读Output Port：向64h发送0d0h命令，然后从60h读取Output Port的内容
    写Output Port：向64h发送0d1h命令，然后向60h写入Output Port的数据
    禁止键盘操作命令：向64h发送0adh
    打开键盘操作命令：向64h发送0aeh

---------------------------------------------------------
    理论上讲，我们只要操作8042芯片的输出端口（64h）的bit 1，就可以控制A20 Gate，但实际上，当你准备向8042的输入缓冲区里写数据时，可能里面还有其它数据没有处理，所以，我们要首先禁止键盘操作，同时等待数据缓冲区中没有数据以后，才能真正地去操作8042打开或者关闭A20 Gate。打开A20 Gate的具体步骤大致如下：

//具体操作步骤
1. 等待8042 Input buffer为空；
2. 发送Write 8042 Output Port （P2）命令到8042 Input buffer；
3. 等待8042 Input buffer为空；
4. 将8042 Output Port（P2）得到字节的第2位置1，然后写入8042 Input buffer

###### ：发送0xd1命令到0x64端口之后，发送0xdf到0x60。

#include <asm.h>

Start the CPU: switch to 32-bit protected mode, jump into C.

The BIOS loads this code from the first sector of the hard disk into

memory at physical address 0x7c00 and starts executing in real mode

with %cs=0 %ip=7c00.

.set PROT_MODE_CSEG,        0x8                     # kernel code segment selector
.set PROT_MODE_DSEG,        0x10                    # kernel data segment selector
.set CR0_PE_ON,             0x1                     # protected mode enable flag
//此处将使能保护模式

###### start address should be 0:7c00, in real mode, the beginning address of the running bootloader
.globl start
start:
.code16                                             # Assemble for 16-bit mode
    cli                                             # Disable interrupts
    cld                                             # String operations increment
    //CLI禁止中断发生
	//STI允许中断发生
//在16位下关闭中断，并设置字符串操作是递增方向
    # Set up the important data segment registers (DS, ES, SS).
    xorw %ax, %ax                                   # Segment number zero
    movw %ax, %ds                                   # -> Data Segment
    movw %ax, %es                                   # -> Extra Segment
    movw %ax, %ss                                   # -> Stack Segment

    # Enable A20:
    #  For backwards compatibility with the earliest PCs, physical
    #  address line 20 is tied low, so that addresses higher than
#  1MB wrap around to zero by default. This code undoes this.

//接下来是A20操作：为了兼容早期的PC机，第20根地址线在实模式下不能使用，所以超过1MB的地址，默认就会返回到地址0，重新从0循环计数，而下面的代码能打开A20地址线

//具体操作步骤
1. 等待8042 Input buffer为空；
2. 发送Write 8042 Output Port （P2）命令到8042 Input buffer；
3. 等待8042 Input buffer为空；
4. 将8042 Output Port（P2）得到字节的第2位置1，然后写入8042 Input buffer
-------------------------------------------------------
bit	meaning
0	output register (60h) 中有数据
1	input register (60h/64h) 有数据
2	系统标志（上电复位后被置为0）
3	data in input register is command (1) or data (0)
4	1=keyboard enabled, 0=keyboard disabled (via switch)
5	1=transmit timeout (data transmit not complete)
6	1=receive timeout (data transmit not complete)
7	1=even parity rec’d, 0=odd parity rec’d (should be odd)
----------------------------------------------------------
*
	有两个端口地址：60h和64h。
    读60h端口，读output buffer
    写60h端口，写input buffer
    读64h端口，读Status Register

    A20 Gate被定义在Output Port的bit 1上
*
seta20.1:   //总体功能，通过将键盘控制器上的A20线置于高电位，全部32条地址线可用，可以访问4G的内存空间

	inb $0x64, %al   //从0x64端口读入一个字节的数据到al（eax寄存器的低8位）
	testb $0x2, %al  //检查第2位是否为1----》》》键盘缓冲区是否为空

理论依据：我们只要操作8042芯片的输出端口（64h）的bit 1，就可以控制A20 Gate，但实际上，当你准备向8042的输入缓冲区里写数据时，可能里面还有其它数据没有处理，所以，我们要首先禁止键盘操作——来自参考书

	jnz seta20.1   //如果上面的测试中发现al的第2位为0（00000010，代表键盘缓冲区为空），就不执行该指令，否则就循环检查（），即等待为空操作
 				//# jnz jump when ZF = 0 即跳出循环
    movb $0xd1, %al     //0xd1 （是d1）->al  
    outb %al, $0x64		//d1->64端口-->向60h写入output port数据

seta20.2:   //继续等待8042键盘控制器不忙
	inb $0x64, %al 		//64->al
	testb $0x2, %al 	//继续判断是否空
    jnz seta20.2        //和之前一样，不忙了就可以出来

    movb $0xdf, %al  	//df->al
        --->;DF为11011111，写入P2，根据图示，A20位置1，开通了 A20 地址线
    outb %al, $0x60     //df->60
    //将al中的数据写入到0x60端口中，将全局描述符表描述符加载到全局描述符表寄存器 ----->现在已经打开A20了
     
    # Switch from real to protected mode, using a bootstrap GDT
    # and segment translation that makes virtual addresses
    # identical to physical addresses, so that the
# effective memory map does not change during the switch.

lgdt gdtdesc  //加载GDT表
引导加载器执行 lgdt指令来把指向 gdt 的指针 gdtdesc加载到全局描述符表（GDT）寄存器中。----查手册
标号gdtdesc是要往GDTR寄存器（里加载的6字节信息，GDTR寄存器的低2字节储存GDT表的长度，高4字节储存GDT表在内存中的首地址.
全局描述符表的是一个保存多个段描述符的“数组”，其起始地址保存在全局描述符表寄存器GDTR中

    movl %cr0, %eax     //cr0->eax
    orl $CR0_PE_ON, %eax    //此处为（见前面）.set CR0_PE_ON,  0x1    # protected mode enable flag
    movl %eax, %cr0         
//使能保护模式
//cr0的第0位为1表示处于保护模式，为0表示处于实时模式，这里将CR0的第0位置1【在这里转换了保护模式】

    # Jump to next instruction, but in 32-bit code segment.
    # Switches processor into 32-bit mode.
    ljmp $PROT_MODE_CSEG, $protcseg
.code32  //长跳转到32位代码段，重装CS、EIP、DS、ES等段寄存器等
 这一段代码就是设置PC和cs寄存器了，
 注意，我们是不可以直接修改CS寄存器的值的，只能通过间接的方式，

    # Set up the protected-mode data segment registers
    # .set PROT_MODE_DSEG,        0x10                    # kernel data segment selector
    movw $PROT_MODE_DSEG, %ax                       # Our data segment selector
    movw %ax, %ds                                   # -> DS: Data Segment
    movw %ax, %es                                   # -> ES: Extra Segment
    movw %ax, %fs                                   # -> FS
    movw %ax, %gs                                   # -> GS
    movw %ax, %ss                                   # -> SS: Stack Segment
    # Set up the stack pointer and call into C. The stack region is from 0--start(0x7c00)
    movl $0x0, %ebp
    movl $start, %esp
    call bootmain   //转到保护模式完成，进入boot主函数
    //这一段设置了一个栈顶指针，然后跳到C代码执行，bootloader的汇编部分至此结束。
    
    # If bootmain returns (it shouldn't), loop. 
spin:
    jmp spin
#这里是一个死循环，如果bootmain函数发生错误返回，计算机就会卡在这儿，就操作系统而言，最好在这里设置一些提示信息

# Bootstrap GDT
.p2align 2                                          # force 4 byte alignment
gdt:        //利用汇编进行的空间申请---静态空间
    SEG_NULLASM                                     # null Segment      --第一个段是空
    SEG_ASM(STA_X|STA_R, 0x0, 0xffffffff)           # code seg for bootloader and kernel
    SEG_ASM(STA_W, 0x0, 0xffffffff)                 # data seg for bootloader and kernel
----->>>>>
。这里一共定义了三项：null，code段和数据段。
我们发现，code段和数据段的基址和限界都是0x0,0xffffffff，
这说明在xv6中，在boot这段时间里，逻辑地址和线性地址实际上是一样的。(还不是页)
#define SEG_ASM(type,base,lim) \
.word (((lim) >> 12) & 0xffff), ((base) & 0xffff); \
.byte (((base) >> 16) & 0xff), (0x90 | (type)), \
(0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)

这个是声明了两个16bit的数据变量和4个8bit的数据变量
段描述符是上下两条 一共64bit
-------<<<<<
--nullasm:
.word 0x0000,0x0000;
.byte 0x00,0x00,0x00,0x00
--for code:
.word 0xFFFF,0x0000;
.byte 0x00,0x9A,0xCF,0x00
--for data
.word 0xFFFF,0x0000;
.byte 0x00,0x92,0xCF,0x00

//只要吧LABEL_GDT放在某段内存的起始位置，跟在它后面的哪些段描述符（内存地址），都可以作为GDT中的元素（或者称为表项），这就是一个表（或者数组）的定义由来。
//全局描述符寄存器GDTR
//GDTR有48位，其中低16位是界限值（全局描述符表减一，因为偏移量是从零开始的），高32位是全局描述符表的基地址；由界限值可以知道能定义最多多少个段描述符，2^16 = 64KB，而每一个段描述符是8个字节，所有64KB/8B = 8K个；

gdtdesc:
    .word 0x17 # 表示的是GDT表的大小                 # sizeof(gdt) - 1
    .long gdt  # 表示的是GDT表的入口地址             # address gdt



-------------------------------------------
http://blog.chinaunix.net/uid-24585655-id-2125527.html
在操作系统的Bootloader过程中，利用
lgdt gdtdesc
    装载全局描述符的基地址和长度进入全局描述符表寄存器，注意加载的时候限长为0表示有1个长度的有效字节
gdtdesc:    .word    0x17            # sizeof(gdt) - 1 23个字节 限长
            .long    gdt            # address gdt  基址
gdt是有三个全局描述符组成，第一个是空描述符---全局描述符表中第一个段描述符设定为空段描述符。
，第二个是代码段描述符，第三个是数据段描述符

gdt:    SEG_NULL                # null seg
        SEG(STA_X|STA_R, 0x0, 0xffffffff)    # code seg
        SEG(STA_W, 0x0, 0xffffffff)     # data seg
其中SEG_NULL和SEG()是两个宏，展开后是利用汇编进行的空间申请

#define SEG_NULL                        \
    .word 0, 0;                        \
    .byte 0, 0, 0, 0
#define SEG(type,base,lim)                    \
    .word (((lim) >> 12) & 0xffff), ((base) & 0xffff);    \
    .byte (((base) >> 16) & 0xff), (0x90 | (type)),        \
        (0xC0 | (((lim) >> 28) & 0xf)), (((base) >> 24) & 0xff)
展开后为

.word 0x0000,0x0000;
.byte 0x00,0x00,0x00,0x00

.word 0xFFFF,0x0000;
.byte 0x00,0x9A,0xCF,0x00

.word 0xFFFF,0x0000;
.byte 0x00,0x92,0xCF,0x00