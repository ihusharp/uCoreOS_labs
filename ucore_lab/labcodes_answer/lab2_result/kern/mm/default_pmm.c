#include <pmm.h>
#include <list.h>
#include <string.h>
#include <default_pmm.h>

/* In the first fit algorithm, the allocator keeps a list of free blocks (known as the free list) and,
   on receiving a request for memory, scans along the list for the first block that is large enough to
   satisfy the request. If the chosen block is significantly larger than that requested, then it is 
   usually split, and the remainder added to the list as another free block.
   Please see Page 196~198, Section 8.2 of Yan Wei Ming's chinese book "Data Structure -- C programming language"
*/
// LAB2 EXERCISE 1: YOUR CODE
// you should rewrite functions: default_init,default_init_memmap,default_alloc_pages, default_free_pages.
/*
 * Details of FFMA
 * (1) Prepare: In order to implement the First-Fit Mem Alloc (FFMA), we should manage the free mem block use some list.
 *              The struct free_area_t is used for the management of free mem blocks. At first you should
 *              be familiar to the struct list in list.h. struct list is a simple doubly linked list implementation.
 *              You should know howto USE: list_init, list_add(list_add_after), list_add_before, list_del, list_next, list_prev
 *              Another tricky method is to transform a general list struct to a special struct (such as struct page):
 *              you can find some MACRO: le2page (in memlayout.h), (in future labs: le2vma (in vmm.h), le2proc (in proc.h),etc.)
 * (2) default_init: you can reuse the  demo default_init fun to init the free_list and set nr_free to 0.
 *              free_list is used to record the free mem blocks. nr_free is the total number for free mem blocks.
 * (3) default_init_memmap:  CALL GRAPH: kern_init --> pmm_init-->page_init-->init_memmap--> pmm_manager->init_memmap
 *              This fun is used to init a free block (with parameter: addr_base, page_number).
 *              First you should init each page (in memlayout.h) in this free block, include:
 *                  p->flags should be set bit PG_property (means this page is valid. In pmm_init fun (in pmm.c),
 *                  the bit PG_reserved is setted in p->flags)
 *                  if this page  is free and is not the first page of free block, p->property should be set to 0.
 *                  if this page  is free and is the first page of free block, p->property should be set to total num of block.
 *                  p->ref should be 0, because now p is free and no reference.
 *                  We can use p->page_link to link this page to free_list, (such as: list_add_before(&free_list, &(p->page_link)); )
 *              Finally, we should sum the number of free mem block: nr_free+=n
 * (4) default_alloc_pages: search find a first free block (block size >=n) in free list and reszie the free block, return the addr
 *              of malloced block.
 *              (4.1) So you should search freelist like this:
 *                       list_entry_t le = &free_list;
 *                       while((le=list_next(le)) != &free_list) {
 *                       ....
 *                 (4.1.1) In while loop, get the struct page and check the p->property (record the num of free block) >=n?
 *                       struct Page *p = le2page(le, page_link);
 *                       if(p->property >= n){ ...
 *                 (4.1.2) If we find this p, then it' means we find a free block(block size >=n), and the first n pages can be malloced.
 *                     Some flag bits of this page should be setted: PG_reserved =1, PG_property =0
 *                     unlink the pages from free_list
 *                     (4.1.2.1) If (p->property >n), we should re-caluclate number of the the rest of this free block,
 *                           (such as: le2page(le,page_link))->property = p->property - n;)
 *                 (4.1.3)  re-caluclate nr_free (number of the the rest of all free block)
 *                 (4.1.4)  return p
 *               (4.2) If we can not find a free block (block size >=n), then return NULL
 * (5) default_free_pages: relink the pages into  free list, maybe merge small free blocks into big free blocks.
 *               (5.1) according the base addr of withdrawed blocks, search free list, find the correct position
 *                     (from low to high addr), and insert the pages. (may use list_next, le2page, list_add_before)
 *               (5.2) reset the fields of pages, such as p->ref, p->flags (PageProperty)
 *               (5.3) try to merge low addr or high addr blocks. Notice: should change some pages's p->property correctly.
 */
free_area_t free_area;
//free_area_t是一个数据结构，记录空闲页个数+链接各个空闲页base的作用
//Page是一个数据结构,包括每个页的属性及双向指针list_entry_t,
	//只有Page空闲,list_entry_t才能插入到free_list中
//list_entry_t是一个数据结构，存储前后指针，

#define free_list (free_area.free_list)
#define nr_free (free_area.nr_free)

// 初始化空闲页块链表
static void
default_init(void) {
    list_init(&free_list);
    nr_free = 0;
}

/*default_init_memmap:  CALL GRAPH: kern_init --> pmm_init-->page_init-->init_memmap--> pmm_manager->init_memmap
 *              This fun is used to init a free block (with parameter: addr_base, page_number).
 *              First you should init each page (in memlayout.h) in this free block, include:
 *                  p->flags should be set bit PG_property (means this page is valid. In pmm_init fun (in pmm.c),
 *                  the bit PG_reserved is setted in p->flags)
 *                  if this page  is free and is not the first page of free block, p->property should be set to 0.
 *                  if this page  is free and is the first page of free block, p->property should be set to total num of block.
 *                  p->ref should be 0, because now p is free and no reference.
 *                  We can use p->page_link to link this page to free_list, (such as: list_add_before(&free_list, &(p->page_link)); )
 *              Finally, we should sum the number of free mem block: nr_free+=n   */
//初始化n个空闲页块
static void
default_init_memmap(struct Page *base, size_t n) {
    assert(n > 0);
    struct Page *p = base;
    for (; p != base + n; p ++) {
        assert(PageReserved(p));//检测p是否已经reserved
        //设置标志位清零（与分配的0没有关系，只是清零
        p->flags = 0;
        /*
         * 将其标记位flag清零
         * ，调用SetPageProperty（kern/mm/memlayout.h，113行）
         *  将flag置1，表示当前页为空。
         */
        SetPageProperty(p);//更改页的状态
        p->property = 0;//页数初始化为0
        set_page_ref(p, 0);	//将映射到此物理页的虚拟页数置为0
        //list_add(&free_list, &(base->page_link));//插入到双向量表
        list_add_before(&free_list, &(p->page_link));
    }
    //对第一页的进行初始化
    base->property = n;
    nr_free += n;//基地址连续空闲页数量加n，且空闲页数量加n。
}

static struct Page *
default_alloc_pages(size_t n) {
    assert(n > 0);
    if (n > nr_free) {
        return NULL;
    }
    //struct Page *page = NULL;
    list_entry_t *le = &free_list;
    list_entry_t *len;

    //双向链表，最后一个指向头指针
    while ((le = list_next(le)) != &free_list) {
    	// convert list entry to page
        struct Page *p = le2page(le, page_link);
        if (p->property >= n) {
        	int i;
        	for(i=0; i<n; i++){
				len = list_next(le);//下一页
				//le2page是通过le的指针，把le对应的页返回出来
				struct Page *pp = le2page(le, page_link);
        		SetPageReserved(pp);//标为预留
        		ClearPageProperty(pp);//清空连续空闲n个页面数量
        		list_del(le);//删除空闲页
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

static void
default_free_pages(struct Page *base, size_t n) {
	//default_free_pages主要完成的是对于页的释放操作，
	//首先有一个assert语句断言这个基地址所在的页是否为预留
	//，如果不是预留页，那么说明它已经是free状态，无法再次free，
	// 也就是之前所述，只有处在占用的页，才能有free操作。
    assert(n > 0);
    assert(PageReserved(base));

    //之后，声明一个页p，p遍历一遍整个物理空间，
    //直到遍历到base所在位置停止，开始释放操作。
    struct Page *p;
    list_entry_t *le = &free_list;
    while((le=list_next(le)) != &free_list){
    	p = le2page(le, page_link);
    	if(p>base)
    		break;
    }//现在le就是base的下一个页对应的指针

    //已经找到位置，开始进行初始化
    for (p = base; p < base + n; p++) {
    	list_add_before(le,&(p->page_link));
    }//从此位置开始，向前面插入释放数量的空页，之前分配的时候删除了

    base->flags = 0;//同开始flags清空
    set_page_ref(base, 0);	//清空引用次数
    ClearPageProperty(base);//清空连续空闲数量
    SetPageProperty(base);	//标记为空闲
    base->property = n;//设置空闲块大小

    p = le2page(le, page_link);
    //此时，p已经到达了插入完释放数量空页的后一个页的位置上。
    //此时，一般会满足base+n==p，因此，尝试向后合并空闲页。
    //如果能合并，那么base的连续空闲页加上p的连续空闲页，
    //且p的连续空闲页置为0,；
    //如果之后的页不能合并，那么p的property一直为0，
    //下面的代码不会对它产生影响。
    if( base+n == p ){//对后面进行合并
    	base->property += p->property;
    	p->property = 0;
    }

    //对前面进行合并
    le = list_prev(&(base->page_link));
    //获取基地址页的前一个页，如果为空，
    //那么循环查找之前所有为空，找能够合并的页
    p = le2page(le, page_link);//跳到前一页的地址处
    if(le!=&free_list && p==base-1){
    	while (le != &free_list) {
    		if(p->property){
    			p->property += base->property;
    			base->property = 0;
    			break;
    		}
    		le = list_prev(le);
    		p = le2page(le,page_link);
    	}
    }
    nr_free += n;
    return;
}

static size_t
default_nr_free_pages(void) {
    return nr_free;
}

static void
basic_check(void) {
    struct Page *p0, *p1, *p2;
    p0 = p1 = p2 = NULL;
    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(p0 != p1 && p0 != p2 && p1 != p2);
    assert(page_ref(p0) == 0 && page_ref(p1) == 0 && page_ref(p2) == 0);

    assert(page2pa(p0) < npage * PGSIZE);
    assert(page2pa(p1) < npage * PGSIZE);
    assert(page2pa(p2) < npage * PGSIZE);

    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));

    unsigned int nr_free_store = nr_free;
    nr_free = 0;

    assert(alloc_page() == NULL);

    free_page(p0);
    free_page(p1);
    free_page(p2);
    assert(nr_free == 3);

    assert((p0 = alloc_page()) != NULL);
    assert((p1 = alloc_page()) != NULL);
    assert((p2 = alloc_page()) != NULL);

    assert(alloc_page() == NULL);

    free_page(p0);
    assert(!list_empty(&free_list));

    struct Page *p;
    assert((p = alloc_page()) == p0);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    free_list = free_list_store;
    nr_free = nr_free_store;

    free_page(p);
    free_page(p1);
    free_page(p2);
}

// LAB2: below code is used to check the first fit allocation algorithm (your EXERCISE 1) 
// NOTICE: You SHOULD NOT CHANGE basic_check, default_check functions!
static void
default_check(void) {
    int count = 0, total = 0;
    list_entry_t *le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        assert(PageProperty(p));
        count ++, total += p->property;
    }
    assert(total == nr_free_pages());

    basic_check();

    struct Page *p0 = alloc_pages(5), *p1, *p2;
    assert(p0 != NULL);
    assert(!PageProperty(p0));

    list_entry_t free_list_store = free_list;
    list_init(&free_list);
    assert(list_empty(&free_list));
    assert(alloc_page() == NULL);

    unsigned int nr_free_store = nr_free;
    nr_free = 0;

    free_pages(p0 + 2, 3);
    assert(alloc_pages(4) == NULL);
    assert(PageProperty(p0 + 2) && p0[2].property == 3);
    assert((p1 = alloc_pages(3)) != NULL);
    assert(alloc_page() == NULL);
    assert(p0 + 2 == p1);

    p2 = p0 + 1;
    free_page(p0);
    free_pages(p1, 3);
    assert(PageProperty(p0) && p0->property == 1);
    assert(PageProperty(p1) && p1->property == 3);

    assert((p0 = alloc_page()) == p2 - 1);
    free_page(p0);
    assert((p0 = alloc_pages(2)) == p2 + 1);

    free_pages(p0, 2);
    free_page(p2);

    assert((p0 = alloc_pages(5)) != NULL);
    assert(alloc_page() == NULL);

    assert(nr_free == 0);
    nr_free = nr_free_store;

    free_list = free_list_store;
    free_pages(p0, 5);

    le = &free_list;
    while ((le = list_next(le)) != &free_list) {
        struct Page *p = le2page(le, page_link);
        count --, total -= p->property;
    }
    assert(count == 0);
    assert(total == 0);
}

const struct pmm_manager default_pmm_manager = {
    .name = "default_pmm_manager",
    .init = default_init,
    .init_memmap = default_init_memmap,
    .alloc_pages = default_alloc_pages,
    .free_pages = default_free_pages,
    .nr_free_pages = default_nr_free_pages,
    .check = default_check,
};


