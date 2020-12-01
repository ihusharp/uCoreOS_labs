EBP是"基址指针"(BASE POINTER), 它最经常被用作高级语言函数调用的"框架指针"(frame pointer). 在破解的时候,经常可以看见一个标准的函数起始代码:

push ebp ;保存当前ebp
mov ebp,esp ;EBP设为当前堆栈指针
sub esp, xxx ;预留xxx字节给函数临时变量.
...

这样一来,EBP 构成了该函数的一个框架, 在EBP上方分别是原来的EBP, 返回地址和参数. EBP下方则是临时变量. 函数返回时作 mov esp,ebp/pop ebp/ret 即可.

ESP 专门用作堆栈指针，被形象地称为栈顶指针，堆栈的顶部是地址小的区域，压入堆栈的数据越多，ESP也就越来越小。在32位平台上，ESP每次减少4字节。

esp：寄存器存放当前线程的栈顶指针
ebp：寄存器存放当前线程的栈底指针
eip：寄存器存放下一个CPU指令存放的内存地址，当CPU执行完当前的指令后，从EIP寄存器中读取下一条指令的内存地址，然后继续执行。
----------------------------------------------------------
函数调用栈的结构
    +|  栈底方向        | 高位地址
     |    ...        |
     |    ...        |
     |  参数3        |
     |  参数2        |
     |  参数1        |
     |  返回地址        |
     |  上一层[ebp]    | <-------- [ebp]
     |  局部变量        |  低位地址


-----------------------------------------------------------
    pushl   %ebp
    movl   %esp , %ebp

这两条汇编指令的含义是：首先将ebp寄存器入栈，然后将栈顶指针esp赋值给ebp。“mov ebp esp”这条指令表面上看是用esp覆盖ebp原来的值，其实不然。因为给ebp赋值之前，原ebp值已经被压栈（位于栈顶），而新的ebp又恰恰指向栈顶。此时ebp寄存器就已经处于一个非常重要的地位，该寄存器中存储着栈中的一个地址（原ebp入栈后的栈顶），从该地址为基准，向上（栈底方向）能获取返回地址、参数值，向下（栈顶方向）能获取函数局部变量值，而该地址处又存储着上一层函数调用时的ebp值。

一般而言，ss:[ebp+4]处为返回地址，ss:[ebp+8]处为第一个参数值（最后一个入栈的参数值，此处假设其占用4字节内存），ss:[ebp-4]处为第一个局部变量，ss:[ebp]处为上一层ebp值。由于ebp中的地址处总是“上一层函数调用时的ebp值”，而在每一层函数调用中，都能通过当时的ebp值“向上（栈底方向）”能获取返回地址、参数值，“向下（栈顶方向）”能获取函数局部变量值。如此形成递归，直至到达栈底。这就是函数调用栈。
------------------------------------------------------------
函数调用时的栈空间：
1、调用者函数把被调函数所需要的参数按照与被调函数的形参顺序相反的顺序压入栈中,即:从右向左依次把被调函数所需要的参数压入栈;
2、调用者函数使用call指令调用被调函数,并把call指令的下一条指令的地址当成返回地址压入栈中(这个压栈操作隐含在call指令中);
3、在被调函数中,被调函数会先保存调用者函数的栈底地址(push ebp),然后再保存调用者函数的栈顶地址,即:当前被调函数的栈底地址(mov ebp,esp);
4、在被调函数中,从ebp的位置处开始存放被调函数中的局部变量和临时变量,并且这些变量的地址按照定义时的顺序依次减小,即:这些变量的地址是按照栈的延伸方向排列的,先定义的变量先入栈,后定义的变量后入栈;
所以,发生函数调用时,入栈的顺序为:
参数N
参数N-1
…..
参数2
参数1
函数返回地址
上一层调用函数的EBP/BP
局部变量1
局部变量2 
---------------------------------------------------------------------
void
print_stackframe(void) {
     /* LAB1 YOUR CODE : STEP 1 */
     /* (1) call read_ebp() to get the value of ebp. the type is (uint32_t);
      * (2) call read_eip() to get the value of eip. the type is (uint32_t);
      * (3) from 0 .. STACKFRAME_DEPTH
      *    (3.1) printf value of ebp, eip
      *    (3.2) (uint32_t)calling arguments [0..4] = the contents in address (unit32_t)ebp +2 [0..4]
      *    (3.3) cprintf("\n");
      *    (3.4) call print_debuginfo(eip-1) to print the C calling function name and line number, etc.
      *    (3.5) popup a calling stackframe
      *           NOTICE: the calling funciton's return addr eip  = ss:[ebp+4]
      *                   the calling funciton's ebp = ss:[ebp]
      */
        uint32_t ebp = read_ebp();
        uint32_t eip = read_eip();
        //(3) from 0 .. STACKFRAME_DEPTH
        int i,j;//这里的c标志不允许for int
        for(i=0; i<STACKFRAME_DEPTH; i++){
        	//(3.1) printf value of ebp, eip
        	cprintf("ebp: 0x%08x  eip: 0x%08x",ebp, eip);
        	//(3.2) (uint32_t)calling arguments [0..4] = the contents in address (unit32_t)ebp +2 [0..4]
        	uint32_t *args = (uint32_t *)ebp+2;
        	cprintf("args: ");
        	for(j=0; j<4; j++){
        		cprintf("0x%08x ", args[j]);
        	}
        	//(3.3) cprintf("\n");
        	cprintf("\n");
        	//(3.4) call print_debuginfo(eip-1) to print the C calling function name and line number, etc
--->>>由于变量eip存放的是下一条指令的地址，因此将变量eip的值减去1，得到的指令地址就属于当前指令的范围了。由于只要输入的地址属于当前指令的起始和结束位置之间，print_debuginfo都能搜索到当前指令，因此这里减去1即可        
        	print_debuginfo(eip-1);//打印eip
        	/*
        	 * (3.5) popup a calling stackframe
        	 * NOTICE: the calling funciton's return addr eip  = ss:[ebp+4]
        	 *         the calling funciton's ebp = ss:[ebp]
        	 */
        	eip = ((uint32_t*)ebp)[1];
        	ebp = ((uint32_t*)ebp)[0];
        }
}
------------->
  uint32_t* args = (uint32_t*)ebp + 2;//指针向后移两位
  //这里的2代表两个整形数地址范围也就是2*4=8
  // uint32_t* args =(uint32_t*)(ebp + 8);
---------------------------------------------------------------
read_eip(void) {
   uint32_t eip;
    /*
    *asm表示后面的代码为汇编代码
    *volatile 表示编译器不要优化代码,后面的指令 保留原样
    *%0表示列表开始的第一个寄存器 
    *“=r”(eip)表示gcc让eip对应一个通用寄存器
    *下面这条语句的作用是将ss:[ebp+4]对应的值保存到eip中，ss:[ebp+4]对应的值是
    *函数的返回地址，也就是说将函数的返回地址保存到eip中，然后返回eip
    */
   asm volatile("movl 4(%%ebp), %0" : "=r" (eip));   
   return eip;
}
---------------------------------------------------------------
阅读bootblock源码可知
    # Set up the stack pointer and call into C. The stack region is from 0--start(0x7c00)
    movl $0x0, %ebp
    7c40:	bd 00 00 00 00       	mov    $0x0,%ebp
    movl $start, %esp
    7c45:	bc 00 7c 00 00       	mov    $0x7c00,%esp
    call bootmain
    7c4a:	e8 82 00 00 00       	call   7cd1 <bootmain>
规定栈底  esp为7c00h
------》》
------》》
打印输出*
Kernel executable memory footprint: 64KB
ebp: 0x00007b08  eip: 0x001009a6args: 0x00010094 0x00000000 0x00007b38 0x00100092 
    kern/debug/kdebug.c:306: print_stackframe+21
ebp: 0x00007b18  eip: 0x00100c9bargs: 0x00000000 0x00000000 0x00000000 0x00007b88 
    kern/debug/kmonitor.c:125: mon_backtrace+10
ebp: 0x00007b38  eip: 0x00100092args: 0x00000000 0x00007b60 0xffff0000 0x00007b64 
    kern/init/init.c:48: grade_backtrace2+33
ebp: 0x00007b58  eip: 0x001000bbargs: 0x00000000 0xffff0000 0x00007b84 0x00000029 
    kern/init/init.c:53: grade_backtrace1+38
ebp: 0x00007b78  eip: 0x001000d9args: 0x00000000 0x00100000 0xffff0000 0x0000001d 
    kern/init/init.c:58: grade_backtrace0+23
ebp: 0x00007b98  eip: 0x001000feargs: 0x001032fc 0x001032e0 0x0000130a 0x00000000 
    kern/init/init.c:63: grade_backtrace+34
ebp: 0x00007bc8  eip: 0x00100055args: 0x00000000 0x00000000 0x00000000 0x00010094 
    kern/init/init.c:28: kern_init+84
ebp: 0x00007bf8  eip: 0x00007d68args: 0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8 
    <unknow>: -- 0x00007d67 --
ebp: 0x00000000  eip: 0x00007c4fargs: 0xf000e2c3 0xf000ff53 0xf000ff53 0xf000ff53 
    <unknow>: -- 0x00007c4e --

由于ebp寄存器指向栈中的位置存放的是调用者的ebp寄存器的值，据此可以继续顺藤摸瓜，不断回溯，直到ebp寄存器的值变为0
---------分析调用关系为：
> kern_init ->
    grade_backtrace ->
        grade_backtrace0(0, (int)kern_init, 0xffff0000) ->
                grade_backtrace1(0, 0xffff0000) ->
                    grade_backtrace2(0, (int)&0, 0xffff0000, (int)&(0xffff0000)) ->
                        mon_backtrace(0, NULL, NULL) ->
                            print_stackframe ->
-----------------------------------------------------------
分析最后一行：
ebp: 0x00007bf8  eip: 0x00007d68 
args: 0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8 
    <unknow>: -- 0x00007d67 --

   1. 发现最后是7bf8， 而栈底本为7c00h
    ebp指向的栈位置存放调用者的ebp寄存器的值，ebp+4指向的栈位置存放返回地址的值，这意味着kern_init函数的调用者（也就是bootmain函数）没有传递任何输入参数给它！因为单是存放旧的ebp、返回地址已经占用8字节了。

在Bootblock.asm中
    7d68:	b8 00 8a ff ff       	mov    $0xffff8a00,%eax
   2.
   一般来说，args存放的4个dword是对应4个输入参数的值。但这里比较特殊，由于bootmain函数调用kern_init并没传递任何输入参数，并且栈顶的位置恰好在boot loader第一条指令存放的地址的上面，而args恰好是kern_int的ebp寄存器指向的栈顶往上第2~5个单元，因此args存放的就是boot loader指令的前16个字节
args: 0xc031fcfa 0xc08ed88e 0x64e4d08e 0xfa7502a8 
   而bootblock如下
00007c00 <start>:
7c00: fa cli
7c01: fc cld
7c02: 31 c0 xor %eax,%eax
7c04: 8e d8 mov %eax,%ds
7c06: 8e c0 mov %eax,%es
7c08: 8e d0 mov %eax,%ss
7c0a: e4 64 in $0x64,%al
7c0c: a8 02 test $0x2,%al
7c0e: 75 fa jne 7c0a <seta20.1>
	注意linux64为小端字节序
---------------------------------------------------
用”call bootmain”转入bootmain函数。
call指令压栈，所以bootmain中ebp为0x7bf8。
栈底两个为bootmain的返回地址+存放bsp的地址共8个字节
