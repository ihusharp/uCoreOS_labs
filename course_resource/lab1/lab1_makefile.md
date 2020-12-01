
https://blog.csdn.net/qq_44768749/article/details/102737825详细lab1
https://www.jianshu.com/p/2f95d38afa1d
相辅相成

Makefile前部分代码是设置环境变量、编译选项等，
其中关键是分别设置了libs和kern目录下的obj文件名，两者合并即为$(KOBJS)。
-----------------------------------------------------
PROJ	:= challenge
EMPTY	:=
SPACE	:= $(EMPTY) $(EMPTY)
SLASH	:= /


##make "V="可输出make执行的命令
V       := @

##选择交叉编译器检查
##$(1) $(2)有点类似于执行shell脚本中的第一个参数和第二个参数...
##makefile 中的$(1) $(2)用来表示call函数传过去的实参,
##call函数原型: $(call variable,param,param,…)

reverse = $(1) $(2)

foo = $(call reverse,a,b)

那么，foo的值就是“a b”。当然，参数的次序是可以自定义的，不一定是顺序的，如：

reverse = $(2) $(1)

foo = $(call reverse,a,b)

此时的foo的值就是“b a”。

-------------------------------

try to infer the correct GCCPREFX

ifndef GCCPREFIX
GCCPREFIX := $(shell if i386-elf-objdump -i 2>&1 | grep '^elf32-i386$$' >/dev/null 2>&1; \
	then echo 'i386-elf-'; \
	elif objdump -i 2>&1 | grep 'elf32-i386' >/dev/null 2>&1; \
	then echo ''; \
	else echo "***" 1>&2; \
	echo "*** Error: Couldn't find an i386-elf version of GCC/binutils." 1>&2; \
	echo "*** Is the directory with i386-elf-gcc in your PATH?" 1>&2; \
	echo "*** If your i386-elf toolchain is installed with a command" 1>&2; \
	echo "*** prefix other than 'i386-elf-', set your GCCPREFIX" 1>&2; \
	echo "*** environment variable to that prefix and run 'make' again." 1>&2; \
	echo "*** To turn off this error, run 'gmake GCCPREFIX= ...'." 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

# try to infer the correct QEMU
----->ifndef防止多重定义
ifndef QEMU
QEMU := $(shell if which qemu-system-i386 > /dev/null; \
	then echo 'qemu-system-i386'; exit; \
	elif which i386-elf-qemu > /dev/null; \
	then echo 'i386-elf-qemu'; exit; \
	else \
	echo "***" 1>&2; \
	echo "*** Error: Couldn't find a working QEMU executable." 1>&2; \
	echo "*** Is the directory containing the qemu binary in your PATH" 1>&2; \
	echo "***" 1>&2; exit 1; fi)
endif

# eliminate default suffix rules
.SUFFIXES: .c .S .h

# delete target files if there is an error (or make is interrupted)
.DELETE_ON_ERROR:

# define compiler and flags

HOSTCC		:= gcc
HOSTCFLAGS	:= -g -Wall -O2

CC		:= $(GCCPREFIX)gcc
CFLAGS	:= -fno-builtin -Wall -ggdb -m32 -gstabs -nostdinc $(DEFS)
CFLAGS	+= $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)
CTYPE	:= c S

LD      := $(GCCPREFIX)ld
LDFLAGS	:= -m $(shell $(LD) -V | grep elf_i386 2>/dev/null)
LDFLAGS	+= -nostdlib

OBJCOPY := $(GCCPREFIX)objcopy
OBJDUMP := $(GCCPREFIX)objdump

COPY	:= cp
MKDIR   := mkdir -p
MV		:= mv
RM		:= rm -f
AWK		:= awk
SED		:= sed
SH		:= sh
TR		:= tr
TOUCH	:= touch -c

OBJDIR	:= obj
BINDIR	:= bin

ALLOBJS	:=
ALLDEPS	:=
TARGETS	:=

include tools/function.mk

listf_cc = $(call listf,$(1),$(CTYPE))

# for cc
add_files_cc = $(call add_files,$(1),$(CC),$(CFLAGS) $(3),$(2),$(4))
create_target_cc = $(call create_target,$(1),$(2),$(3),$(CC),$(CFLAGS))

# for hostcc
add_files_host = $(call add_files,$(1),$(HOSTCC),$(HOSTCFLAGS),$(2),$(3))
create_target_host = $(call create_target,$(1),$(2),$(3),$(HOSTCC),$(HOSTCFLAGS))

cgtype = $(patsubst %.$(2),%.$(3),$(1))
objfile = $(call toobj,$(1))
asmfile = $(call cgtype,$(call toobj,$(1)),o,asm)
outfile = $(call cgtype,$(call toobj,$(1)),o,out)
symfile = $(call cgtype,$(call toobj,$(1)),o,sym)

# for match pattern
match = $(shell echo $(2) | $(AWK) '{for(i=1;i<=NF;i++){if(match("$(1)","^"$$(i)"$$")){exit 1;}}}'; echo $$?)

# >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
# include kernel/user

INCLUDE	+= libs/

CFLAGS	+= $(addprefix -I,$(INCLUDE))

LIBDIR	+= libs

$(call add_files_cc,$(call listf_cc,$(LIBDIR)),libs,)

--->
 1. 调用了add_files_cc函数，
输入参数有2个，第1个是调用另一个函数listf_cc的返回值,第2个是libs（目录名）
add_files_cc = $(call add_files,$(1),$(CC),$(CFLAGS) $(3),$(2),$(4))
listf_cc = $(call listf,$(1),$(CTYPE))
CTYPE	:= c S
 2. listf_cc函数的定义为listf_cc = $(call listf,$(1),$(CTYPE))
可见listf_cc又调用了listf函数，
调用时传入的第1个参数为$(1) = $(LIBDIR) = libs，第2个参数为$(CTYPE) = c S
 3. listf函数的定义为listf = $(filter $(if $(2),$(addprefix %.,$(2)),%), $(wildcard $(addsuffix $(SLASH)*,$(1))))，
将输入参数代入得：listf = $(filter %.c %.S, libs/*)
可见此处调用listf的返回结果为libs目录下的所有.c和.S文件。
由于lab1的libs目录下只有.h和.c文件，因此最终返回.c文件
 4. 因此$(call add_files_cc,$(call listf_cc,$(LIBDIR)),libs,)
可简化为   --->  add_files_cc(libs/*.c, libs)
 5. add_files_cc的定义为
 add_files_cc = $(call add_files,$(1),$(CC),$(CFLAGS) $(3),$(2),$(4))
 结合4可化简为 add_files(libs/*.c, gcc, $(CFLAGS), libs)
 6. 6.add_files的定义为
 add_files = $(eval $(call do_add_files_to_packet,$(1),$(2),$(3),$(4),$(5)))
    其中do_add_files_to_packet::
# define do_add_files_to_packet
# __temp_packet__ := $(call packetname,$(4))
# ifeq ($$(origin $$(__temp_packet__)),undefined)
# $$(__temp_packet__) :=
# endif
# __temp_objs__ := $(call toobj,$(1),$(5))
# $$(foreach f,$(1),$$(eval $$(call cc_template,$$(f),$(2),$(3),$(5))))
# $$(__temp_packet__) += $$(__temp_objs__)
# endef

 7. call packetname
    packetname的定义为
    packetname = $(if $(1),$(addprefix $(OBJPREFIX),$(1)),$(OBJPREFIX))，
    其中$(OBJPREFIX)=__objs_，
    而$(1)=libs，因此__temp_packet_ = __objs_libs
 8. 综上，$(call add_files_cc,$(call listf_cc,$(LIBDIR)),libs,)
 最终效果是__objs_libs = obj/libs/**/*.o
-----
*含义是寻找libs目录下的所有具有.c, .s后缀的文件，并生成相应的.o文件，
放置在obj/libs/文件夹下，具体生成的文件是printfmt.o, string.o文件，
与此同时，该文件夹下还生成了.d文件，
这是Makefile自动生成的依赖文件列表所存放的位置，比如打开string.d文件可以发现，
string.o文件的生成依赖于string.c, string.h, x86.h, defs.h四个文件，
这与我们对于代码的观察是一致的；这部分编译所使用的编译选项保存在CFLAGS变量下，
关于具体每一个使用到的gcc编译选项的含义，
将在下文具体分析Makefile中定义CFLAGS变量的部分进行详细描述；
*

# -------------------------------------------------------------------
# kernel

KINCLUDE	+= kern/debug/ \
			   kern/driver/ \
			   kern/trap/ \
			   kern/mm/

KSRCDIR		+= kern/init \
			   kern/libs \
			   kern/debug \
			   kern/driver \
			   kern/trap \
			   kern/mm

KCFLAGS		+= $(addprefix -I,$(KINCLUDE))

$(call add_files_cc,$(call listf_cc,$(KSRCDIR)),kernel,$(KCFLAGS))
# 同上述分析 生成libs目录下的obj文件名
# 此步 实际效果是__objs_kernel = obj/kern/**/*.o


KOBJS	= $(call read_packet,kernel libs)
---------------------------------------------
# create kernel target  生成kernel代码


  -----***总的来说：

表示/bin/kernel文件依赖于tools/kernel.ld文件，并且没有指定生成规则，

也就是说如果没有预先准备好kernel.ld，就会在make的时候产生错误；

之后的$(kernel): $(KOBJS)

表示kernel文件的生成还依赖于上述生成的obj/libs, obj/kernels下的.o文件，

并且生成规则为使用ld链接器将这些.o文件连接成kernel文件，

其中ld的-T表示指定使用kernel.ld来替代默认的链接器脚本；

关于LDFLAGS中的选项含义，将在下文中描述LDFLAGS变量定义的时候进行描述；

之后还使用objdump反汇编出kernel的汇编代码，-S表示将源代码与汇编代码混合展示出来，

这部分代码最终保存在kernel.asm文件中；	-t表示打印出文件的符号表表项，

然后通过管道将带有符号表的反汇编结果作为sed命令的标准输入进行处理，

最终将符号表信息保存到kernel.sym文件中；

kernel = $(call totarget,kernel)
---->

totarget = $(addprefix $(BINDIR)$(SLASH),$(1))

BINDIR  := bin

SLASH   := /

即生成目标名为 bin/kernel

------------------------

$(kernel): tools/kernel.ld
//生成kernel目标文件需要依赖于链接配置文件tools/kernel.ld

--->.ld做连接器脚本

而kernel.ld文件是一个链接脚本，其中设置了输出的目标文件的入口地址及各个段的一些属性，

包括各个段是由输入文件的哪些段组成、各个段的起始地址等。

//指出kernel目标文件依赖的obj文件。
//最终效果为KOBJS=obj/libs/.o obj/kern/**/.o
//即需要kern 和 libs下的obj文件
//对应开头的生成两个文件夹下的obj文件
$(kernel): $(KOBJS)	 //kernel的生成还依赖KOBJS

KOBJS   = $(call read_packet,kernel libs)

read_packet = $(foreach p,$(call packetname,$(1)),$($(p)))

packetname = $(if $(1),$(addprefix $(OBJPREFIX),$(1)),$(OBJPREFIX))

OBJPREFIX   := __objs_

	@echo + ld $@    //$@代表目标文件 将以下文件和目标文件链接起来
# 	// output: `+ ld bin/kernel`
# 	V       := @
#   LD      := $(GCCPREFIX)ld
	$(V)$(LD) $(LDFLAGS) -T tools/kernel.ld -o $@ $(KOBJS)
	//链接obj/libs/*和obj/kernel/init/*...所有的目标文件，使用kernel.ld做连接器脚本
	//以得到kernel文件
	
	@$(OBJDUMP) -S $@ > $(call asmfile,kernel)
	//使用objdump工具对kernel目标文件反汇编，以便后续调试。
	
	@$(OBJDUMP) -t $@ | $(SED) '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(call symfile,kernel)
	//最终的内核文件去除符号表等信息，并输出符号表信息，汇编文件信息，和输出信息
	//等效于objdump -t bin/kernel > obj/kernel.sym。

$(call create_target,kernel)
# 调用create_target函数：$(call create_target,kernel)，
# 而create_target的定义为
# create_target = $(eval $(call do_create_target,$(1),$(2),$(3),$(4),$(5)))
# 可见create_target只是进一步调用了do_create_target的函数：
----->>>>>do_create_target(kernel)
# -------------------------------------------------------------------

# create bootblock
//为了生成bootblock，首先应生成bootasm.o,bootmain.c和sign
bootfiles = $(call listf_cc,boot)
# 前面已经知道listf_cc函数是过滤出对应目录下的.c和.S文件，
# 因此等同于bootfiles=boot/\*.c  boot/\*.S

$(foreach f,$(bootfiles),$(call cc_compile,$(f),$(CC),$(CFLAGS) -Os -nostdinc))
--->编译bootfiles生成.o文件
---->下面进行与kernel文件生成差不多的步骤
bootblock = $(call totarget,bootblock)
# 前面已经知道totarget函数是给输入参数增加前缀"bin/"，
# 因此等同于bootblock="bin/bootblock"

$(bootblock): $(call toobj,$(bootfiles)) | $(call totarget,sign)
# 	.声明bin/bootblock依赖于obj/boot/*.o 和bin/sign文件
# 	注意toobj函数的作用是给输入参数增加前缀obj/，并将文件后缀名改为.o
	@echo + ld $@  //进行链接
	$(V)$(LD) $(LDFLAGS) -N -e start -Ttext 0x7C00 $^ -o $(call toobj,bootblock)
	//链接依赖的.o文件，使用kernel.ld做连接器脚本
	//以得到kernel文件	
#     -N：将代码段和数据段设置为可读可写；
#     -e：设置入口；
#     -Ttext：设置起始地址为0X7C00；

	@$(OBJDUMP) -S $(call objfile,bootblock) > $(call asmfile,bootblock)
	//使用objdump将编译结果反汇编出来，保存在bootclock.asm中，-S表示将源代码与汇编代码混合表示；
	@$(OBJDUMP) -t $(call objfile,bootblock) | $(SED) '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(call symfile,bootblock)
	
	@$(OBJCOPY) -S -O binary $(call objfile,bootblock) $(call outfile,bootblock)
	// 使用objcopy将bootblock.o二进制拷贝到bootblock.ou
	@$(call totarget,sign) $(call outfile,bootblock) $(bootblock)
	//使用sign程序, 利用bootblock.out生成bootblock;

$(call create_target,bootblock)

# -------------------------------------------------------------------

# 读tools/sign.c代码可知
就是将输入文件拷贝至输出，并控制大小为512（char buf[512]）
    buf[510] = 0x55;
    buf[511] = 0xAA;
    可见将最后两个字节设为 55AA
**    也就是ELF文件的magic number

小知识：AA展开为10101010,55展开为01010101,

变成串行电平的话就是一个占空比为50%的方波，

这种方波在电路中最容易被分辨是否受干扰或者畸变，

在实际波形的观察中也最容易看出毛病所在，

----
create 'sign' tools

$(call add_files_host,tools/sign.c,sign,sign)
//生成obj/sign/tools/sign.o
$(call create_target_host,sign,sign)
//利用sign.o生成sign，
//--------------------------至此bootblock所依赖的文件均生成完毕；------
生成sign的代码

1.调用了add_files_host函数：$(call add_files_host, tools/sign.c, sign, sign)

2.add_files_host的定义为

#add_files_host = $(call add_files,$(1),$(HOSTCC),$(HOSTCFLAGS),$(2),$(3))

调用add_files函数：add_files(tools/sign.c, gcc, $(HOSTCFLAGS), sign, sign)

3.add_files的定义为add_files = $(eval $(call do_add_files_to_packet,$(1),$(2),$(3),$(4),$(5)))，

#根据前面的分析，do_add_files_to_packet的作用是生成obj文件，
#因此这里调用add_files的作用是设置\_\_objs\_sign = obj/sign/tools/sign.o

4.调用了create_target_host函数：$(call create_target_host,sign,sign)

5.ate_target_host的定义为create_target_host = $(call create_target,$(1),$(2),$(3),$(HOSTCC),$(HOSTCFLAGS))可见

#是调用了create_target函数：create_target(sign, sign, gcc, $(HOSTCFLAGS))

6.create_target的定义为create_target = $(eval $(call do_create_target,$(1),$(2),$(3),$(4),$(5)))。

#根据前面的分析，do_create_target的作用是生成目标文件，
#因此这里调用create_target的作用是生成obj/sign/tools/sign.o

-------------------------------------------------------------------

##### create ucore.img

UCOREIMG	:= $(call totarget,ucore.img)
//此时通过call函数来实现创建ucore.img的过程，
// UCOREIMG代表的就是即将生成的ucore.img文件 

$(UCOREIMG): $(kernel) $(bootblock)
 	//这里表示ucore-img文件的生成依赖于kernel和bootblock 
*
	$(V)dd if=/dev/zero of=$@ count=10000  
	//为UCOREIMG分配一个10000*512字节大小的空间 即5000M 约5g

备注：在类UNIX 操作系统中, /dev/zero 是一个特殊的文件，

当你读它的时候，它会提供无限的空字符(NULL, ASCII NUL, 0x00)。

其中的一个典型用法是用它提供的字符流来覆盖信息，

另一个常见用法是产生一个特定大小的空白文件。

BSD就是通过mmap把/dev/zero映射到虚地址空间实现共享内存的。

可以使用mmap将/dev/zero映射到一个虚拟的内存空间，

这个操作的效果等同于使用一段匿名的内存（没有和任何文件相关）。

	$(V)dd if=$(bootblock) of=$@ conv=notrunc 
	//将bootblock拷贝到UCOREIMG中，大小为512字节
	
	$(V)dd if=$(kernel) of=$@ seek=1 conv=notrunc 
	//将kernel拷贝到UCOREIMG中，从文件开头跳过seek个块之后开始拷贝
	//此时seek=1，即将第一个512字节用于保存bootblock
	//从512后开始拷贝kern
dd 

使用权限: 所有使用者dd 这个指令在 manual 里的定义是 convert and copy a file

dd [选项]

if =输入文件（或设备名称）。

of =输出文件（或设备名称）。

conv = notrunc 不截短输出文件。

count=blocks 只拷贝输入的blocks块。 

$(call create_target,ucore.img)
over！！！！！！！！！！！！！！！！！！！！！！！

>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

$(call finish_all)

IGNORE_ALLDEPS	= clean \
				  dist-clean \
				  grade \
				  touch \
				  print-.+ \
				  handin

ifeq ($(call match,$(MAKECMDGOALS),$(IGNORE_ALLDEPS)),0)
-include $(ALLDEPS)
endif

-----------------

files for grade script

TARGETS: $(TARGETS)
all: $(TARGETS)
.DEFAULT_GOAL := TARGETS

.PHONY: qemu qemu-nox debug debug-nox
lab1-mon: $(UCOREIMG)
	$(V)$(TERMINAL) -e "$(QEMU) -S -s -d in_asm -D $(BINDIR)/q.log -monitor stdio -hda $< -serial null"
	$(V)sleep 2
	$(V)$(TERMINAL) -e "gdb -q -x tools/lab1init"
debug-mon: $(UCOREIMG)

$(V)$(QEMU) -S -s -monitor stdio -hda $< -serial null &

	$(V)$(TERMINAL) -e "$(QEMU) -S -s -monitor stdio -hda $< -serial null"
	$(V)sleep 2
	$(V)$(TERMINAL) -e "gdb -q -x tools/moninit"
qemu-mon: $(UCOREIMG)
	$(V)$(QEMU) -monitor stdio -hda $< -serial null
qemu: $(UCOREIMG)
	$(V)$(QEMU) -parallel stdio -hda $< -serial null

qemu-nox: $(UCOREIMG)
	$(V)$(QEMU) -serial mon:stdio -hda $< -nographic
TERMINAL        :=gnome-terminal
gdb: $(UCOREIMG)
	$(V)$(QEMU) -S -s -parallel stdio -hda $< -serial null
debug: $(UCOREIMG)
	$(V)$(QEMU) -S -s -parallel stdio -hda $< -serial null &
	$(V)sleep 2
	$(V)$(TERMINAL)  -e "cgdb -q -x tools/gdbinit"
	
debug-nox: $(UCOREIMG)
	$(V)$(QEMU) -S -s -serial mon:stdio -hda $< -nographic &
	$(V)sleep 2
	$(V)$(TERMINAL) -e "gdb -q -x tools/gdbinit"

.PHONY: grade touch

GRADE_GDB_IN	:= .gdb.in
GRADE_QEMU_OUT	:= .qemu.out
HANDIN			:= proj$(PROJ)-handin.tar.gz

TOUCH_FILES		:= kern/trap/trap.c

MAKEOPTS		:= --quiet --no-print-directory

grade:
	$(V)$(MAKE) $(MAKEOPTS) clean
	$(V)$(SH) tools/grade.sh

touch:
	$(V)$(foreach f,$(TOUCH_FILES),$(TOUCH) $(f))

print-%:
	@echo $($(shell echo $(patsubst print-%,%,$@) | $(TR) [a-z] [A-Z]))

.PHONY: clean dist-clean handin packall
clean:
	$(V)$(RM) $(GRADE_GDB_IN) $(GRADE_QEMU_OUT)
	-$(RM) -r $(OBJDIR) $(BINDIR)

dist-clean: clean
	-$(RM) $(HANDIN)

handin: packall
	@echo Please visit http://learn.tsinghua.edu.cn and upload $(HANDIN). Thanks!

packall: clean
	@$(RM) -f $(HANDIN)
	@tar -czf $(HANDIN) `find . -type f -o -type d | grep -v '^\.*$$' | grep -vF '$(HANDIN)'`
