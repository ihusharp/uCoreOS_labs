同样，我们所实现的函数，也是通过名字绑定的，具体的绑定过程在（kern/mm/pmm.c，285+行）：

    const struct pmm_manager default_pmm_manager = {
        .name = "default_pmm_manager",
        .init = default_init,
        .init_memmap = default_init_memmap,
        .alloc_pages = default_alloc_pages,
        .free_pages = default_free_pages,
        .nr_free_pages = default_nr_free_pages,
        .check = default_check,
    };
----------------------------------------------------------------------
static void
page_init(void) {
struct e820map *memmap = (struct e820map *)(0x8000 + KERNBASE);* 
//首先声明一个e820map类的对象memmap，与物理内存相关，在本实验中，我们获取内存信息的方式是通过e820中断（一种特殊的内核中断模式）
    uint64_t maxpa = 0;

    cprintf("e820map:\n");
    int i;
for (i = 0; i < memmap->nr_map; i ++) {		
//这里可以看做一个遍历，第一轮遍历是遍历物理地址空间，获取物理地址的最大值maxpa。（探测物理内存布局）
        uint64_t begin = memmap->map[i].addr, end = begin + memmap->map[i].size;
        cprintf("  memory: %08llx, [%08llx, %08llx], type = %d.\n",
                memmap->map[i].size, begin, end - 1, memmap->map[i].type);
        if (memmap->map[i].type == E820_ARM) {
            if (maxpa < end && begin < KMEMSIZE) {
                maxpa = end;
            }
        }
    }
    if (maxpa > KMEMSIZE) {
        maxpa = KMEMSIZE;
    }		//maxpa不能大于所允许的最大值，这个最大值宏定义在memlayout的第57行，物理地址所允许的最大值为0x38000000



13420536/4096 = 32766个页，是最大物理内存分页的页数

13420536b=128MB  大概物理地址的最大值为128MB

```
【小知识】即896MB = 0x1 C000 0000‬，与之前说到的0x1 0000 0000映射到0x1 C000 0000相对应‬‬

进程可以寻址4G，其中0~3G为用户态，3G~4G为内核态。如果内存不超过1G那么最后这1G线性空间足够映射物理内存了，如果物理内存大于1G，为了使内核空间的1G线性地址可以访问到大于1G的物理内存，把物理内存分为两部分，0~896MB的进行直接内存映射，也就是说存在一个线性关系：virtual address=physical address+
PAGE_OFFSET，这里的PAGE_OFFSET为3G。还剩下一个128MB的空间，这个空间作为一个窗口动态进行映射，这样就可以访问大于1G的内存，但是同一时刻内核空间还是只有1G的线性地址，只是不同时刻可以映射到不同的地方。综上，大于896MB的物理内存就是高端内存，内核引入高端内存这个概念是为了通过128MB这个窗口访问大于1G的物理内存。
```

​	实验三可以保存262144/8=32768个页，即128MB的内存空间。

```
   也就是说
    npage = maxpa / PGSIZE;
    
    cprintf("  npage: %d \n",npage);
    cprintf("  KMEMSIZE: %d \n",KMEMSIZE);
    cprintf("  maxpa: %d \n",maxpa);
    cprintf("  number%d is maxpa \n",j);

  page: 32766 
  KMEMSIZE: 939524096 
  maxpa: 134209536 
  number3 is maxpa 
```



    extern char end[];
     
    npage = maxpa / PGSIZE;		//创建的页数量等于物理地址最大值除以页大小，其中页大小为4096字节，即4KB，该定义在mmu.h的第226行。
    pages = (struct Page *)ROUNDUP((void *)end, PGSIZE);
     
    for (i = 0; i < npage; i ++) {
        SetPageReserved(pages + i);
    }	//将所有的页设置为保留页，在实际初始化页面init_memmap的时候，又会更改回非保留，推测是在初始化过程中这样处理，是为了防止页面被分配，结构被破坏。
     
    uintptr_t freemem = PADDR((uintptr_t)pages + sizeof(struct Page) * npage);
     
    for (i = 0; i < memmap->nr_map; i ++) {
        uint64_t begin = memmap->map[i].addr, end = begin + memmap->map[i].size;
        if (memmap->map[i].type == E820_ARM) {	//第二次遍历物理内存，这一次遍历，主要是调用init_memmap初始化各个页表。
            if (begin < freemem) {
                begin = freemem;
            }
            if (end > KMEMSIZE) {
                end = KMEMSIZE;
            }
            if (begin < end) {
                begin = ROUNDUP(begin, PGSIZE);
                end = ROUNDDOWN(end, PGSIZE);
                if (begin < end) {
                    init_memmap(pa2page(begin), (end - begin) / PGSIZE);
                }
            }
        }
    }
}
page_init函数主要是完成了一个整体物理地址的初始化过程，包括设置标记位，探测物理内存布局等操作。上面函数的注释中，标出了几个关键位置代码。


但是，其中最关键的部分，也是和实验相关的页初始化，交给了init_memmap函数处理。
pa2page:返回传入参数pa开始的第一个物理页，其基地址base。
end-begin / PGSIZE:物理页的个数。
---------------------------------------------------------------------------
1、pa2page（begin）：（kern/mm/pmm.h，87——93行）
static inline struct Page *

pa2page(uintptr_t pa) {
    if (PPN(pa) >= npage) {
        panic("pa2page called with invalid pa");
    }
    return &pages[PPN(pa)];

}

其中PPN是物理地址页号，
该函数的作用是，返回传入参数pa开始的第一个物理页，其基地址base。
-----
end-begin / PGSIZE
由于end和begin都是循环中记录位置的标记，PGSIZE为4KB的页大小，这里就代表物理页的个数。
---------------------------------------------------------------------
static void
default_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(PageReserved(p));//检测p是否已经reserved
        //设置标志位
        p->flags = p->property = 0;
        set_page_ref(p, 0);
    }
    //对第一页的进行初始化
    base->property = n;
    SetPageProperty(base);
    nr_free += n;
    list_add(&free_list, &(base->page_link));
}*

我们可以看出来，从上一轮传过来的参数，是从基地址开始的第一个物理页和待初始化的物理页的数量。

 

首先，用一个页结构p，存储传下来的base页面，之后对于紧随其后的n个页面进行遍历判断是否为预留页（之前，因为防止初始化页面被分配或破坏，已经设置了预留页），如果这里已经设置为了预留页（assert断言），那么就可以对它进行初始化：

 

1、将其标记位flag清零，调用SetPageProperty（kern/mm/memlayout.h，113行）将flag置1，表示当前页为空。（应该是一个可以优化的点）

#define SetPageProperty(page)       set_bit(PG_property, &((page)->flags))

 

其中，PG_property的定义在（kern/mm/memlayout.h，108行）

#define PG_property                 1       // the member 'property' is valid

 

2、将其连续空页数量设置为0，即p->property。

 

3、映射到此物理页的虚拟页数量置为0，调用set_page_ref函数（kern/mm/pmm.h，123——126行）

set_page_ref(struct Page *page, int val) {

    page->ref = val;

}

 

4、插入到双向链表中，free_list因为宏定义的原因，指的是free_area_t中的list结构。

5、基地址连续空闲页数量加n，且空闲页数量加n。
----------------------------------------------------------------------
static struct Page *
default_alloc_pages(size_t n) {
    assert(n > 0);
    if (n > nr_free) {
        return NULL;
    }
    list_entry_t *le, *len;
    le = &free_list;

    while((le=list_next(le)) != &free_list) {//寻找一个可分配的连续页
      struct Page *p = le2page(le, page_link);
      if(p->property >= n){
        int i;
        for(i=0;i<n;i++){
          len = list_next(le);
          struct Page *pp = le2page(le, page_link);
          SetPageReserved(pp);
          ClearPageProperty(pp);
          list_del(le);//删链表
          le = len;
        }
        if(p->property>n){
          (le2page(le,page_link))->property = p->property - n;
        }
        ClearPageProperty(p);
        SetPageReserved(p);
        nr_free -= n;
        return p;
      }
    }
    return NULL;
}

lloc_page，这个函数是用来分配空闲页的。

首先判断空闲页的大小是否大于所需的页块大小。

 

如果需要分配的页面数量n，已经大于了空闲页的数量，那么直接return NULL分配失败。

 

过了这一个检查之后，遍历整个空闲链表。如果找到合适的空闲页，即p->property >= n（从该页开始，连续的空闲页数量大于n），即可认为可分配，重新设置标志位。具体操作是调用SetPageReserved(pp)和ClearPageProperty(pp)，设置当前页面预留，以及清空该页面的连续空闲页面数量值。

然后从空闲链表，即free_area_t中，记录空闲页的链表，删除此项。

如果当前空闲页的大小大于所需大小。则分割页块。具体操作就是，刚刚分配了n个页，如果分配完了，还有连续的空间，则在最后分配的那个页的下一个页（未分配），更新它的连续空闲页值。如果正好合适，则不进行操作。

最后计算剩余空闲页个数并返回分配的页块地址。

----------------------------------------