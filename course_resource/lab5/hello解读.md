```
    ……
    + cc user/hello.c
    gcc -Iuser/ -fno-builtin -Wall -ggdb -m32 -gstabs -nostdinc  -fno-stack-protector -Ilibs/ -Iuser/include/ -Iuser/libs/ -c user/hello.c -o obj/user/hello.o
    ld -m    elf_i386 -nostdlib -T tools/user.ld -o obj/__user_hello.out  obj/user/libs/initcode.o obj/user/libs/panic.o obj/user/libs/stdio.o obj/user/libs/syscall.o obj/user/libs/ulib.o obj/user/libs/umain.o  obj/libs/hash.o obj/libs/printfmt.o obj/libs/rand.o obj/libs/string.o obj/user/hello.o
    ……
    ld -m    elf_i386 -nostdlib -T tools/kernel.ld -o bin/kernel  obj/kern/init/entry.o obj/kern/init/init.o …… -b binary …… obj/__user_hello.out
    ……
```

从中可以看出，hello应用程序不仅仅是hello.c，还包含了支持hello应用程序的用户态库：

- user/libs/initcode.S：所有应用程序的起始用户态执行地址“_start”，调整了EBP和ESP后，调用umain函数。
- user/libs/umain.c：实现了umain函数，这是所有应用程序执行的第一个C函数，它将调用应用程序的main函数，并在main函数结束后调用exit函数，而exit函数最终将调用sys_exit系统调用，让操作系统回收进程资源。
- user/libs/ulib.[ch]：实现了最小的C函数库，除了一些与系统调用无关的函数，其他函数是对访问系统调用的包装。
- user/libs/syscall.[ch]：用户层发出系统调用的具体实现。
- user/libs/stdio.c：实现cprintf函数，通过系统调用sys_putc来完成字符输出。
- user/libs/panic.c：实现__panic/__warn函数，通过系统调用sys_exit完成用户进程退出。

除了这些用户态库函数实现外，还有一些libs/*.[ch]是操作系统内核和应用程序共用的函数实现。这些用户库函数其实在本质上与UNIX系统中的标准libc没有区别，只是实现得很简单，但hello应用程序的正确执行离不开这些库函数。

【注意】libs/*.[ch]、user/libs/*.[ch]、user/*.[ch]的源码中没有任何特权指令。

在make的最后一步执行了一个ld命令，把hello应用程序的执行码obj/__user_hello.out连接在了ucore  kernel的末尾。且ld命令会在kernel中会把__user_hello.out的**位置和大小**记录在

全局变量**_ binary_obj___user_ hello_out_start和_ binary_obj___user_hello_out_size**中，这样这个hello用户程序就能够和ucore内核一起被 bootloader  加载到内存里中，**并且通过这两个全局变量定位hello用户程序执行码的起始位置和大小。**而到了与文件系统相关的实验后，ucore会提供一个简单的文件系统，那时所有的用户程序就都不再用这种方法进行加载了，而可以用大家熟悉的文件方式进行加载了。

在tools/user.ld描述了用户程序的用户虚拟空间的执行入口虚拟地址：

```
    SECTIONS {
        /* Load programs at this address: "." means the current address */
        . = 0x800020;
```

在tools/kernel.ld描述了操作系统的内核虚拟空间的起始入口虚拟地址：

```
    SECTIONS {
        /* Load the kernel at this address: "." means the current address */
        . = 0xC0100000;
```

这样ucore把用户进程的虚拟地址空间分了两块，一块与内核线程一样，是所有用户进程都共享的内核虚拟地址空间，映射到同样的物理内存空间中，这样在物理内存中只需放置一份内核代码，使得用户进程从用户态进入核心态时，内核代码可以统一应对不同的内核程序；另外一块是用户虚拟地址空间，虽然虚拟地址范围一样，但映射到不同且没有交集的物理内存空间中。这样当ucore把用户进程的执行代码（即应用程序的执行代码）和数据（即应用程序的全局变量等）放到用户虚拟地址空间中时，确保了各个进程不会“非法”访问到其他进程的物理内存空间。