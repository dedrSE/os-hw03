// Physical memory allocator, intended to allocate
// memory for user processes, kernel stacks, page table pages,
// and pipe buffers. Allocates 4096-byte pages.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"
#include "physmeminfo.h"	//physframe 관련 정의 파일 헤더
#include "proc.h"		//myproc() 함수

void freerange(void *vstart, void *vend);
extern char end[]; // first address after kernel loaded from ELF file
                   // defined by the kernel linker script in kernel.ld
//main.c -> proc.c userinit시 1로 변경
extern int is_user_process_started;
//ticks 전역변수 extern
extern uint ticks;

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  int use_lock;
  struct run *freelist;
} kmem;

//전역 프레임 테이블
struct physframe_info pf_info[PFNNUM];

// Initialization happens in two phases.
// 1. main() calls kinit1() while still using entrypgdir to place just
// the pages mapped by entrypgdir on free list.
// 2. main() calls kinit2() with the rest of the physical pages
// after installing a full page table that maps them on all cores.
void
kinit1(void *vstart, void *vend)
{
  initlock(&kmem.lock, "kmem");
  kmem.use_lock = 0;
  freerange(vstart, vend);
}

void
kinit2(void *vstart, void *vend)
{
  freerange(vstart, vend);
  kmem.use_lock = 1;
}

void
freerange(void *vstart, void *vend)
{
  char *p;
  p = (char*)PGROUNDUP((uint)vstart);
  for(; p + PGSIZE <= (char*)vend; p += PGSIZE)
    kfree(p);
}
//PAGEBREAK: 21
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(char *v)
{
  struct run *r;

  if((uint)v % PGSIZE || v < end || V2P(v) >= PHYSTOP)
    panic("kfree");



  // Fill with junk to catch dangling refs.
  memset(v, 1, PGSIZE);

  if(kmem.use_lock)
    acquire(&kmem.lock);
  
  //(메모리 포인트 - 커널주소) / 페이지 사이즈(4KB)
  //락 획득 후 작업
  uint frame_index = ((uint)v - KERNBASE) / PGSIZE;
  if(frame_index < PFNNUM) {
	  pf_info[frame_index].allocated = 0;			// free
	  pf_info[frame_index].pid = -1;			// 소유 프로세스 없음
	  pf_info[frame_index].start_tick = 0;			// 시간 초기화
	  pf_info[frame_index].frame_index = frame_index;	// 자기 인덱스 기록
  }
  
  r = (struct run*)v;
  r->next = kmem.freelist;
  kmem.freelist = r;
  if(kmem.use_lock)
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
char*
kalloc(void)
{
  struct run *r;

  if(kmem.use_lock)
    acquire(&kmem.lock);
  r = kmem.freelist;
  if(r){
    kmem.freelist = r->next;
    uint frame_index = ((uint)r - KERNBASE) / PGSIZE;
    //userinit 이후부터 작업
    if(is_user_process_started && (frame_index < PFNNUM)) {
	    struct proc *p = myproc();
	    pf_info[frame_index].allocated = 1;
	    pf_info[frame_index].pid = p ? p->pid : -1;
	    pf_info[frame_index].start_tick = ticks;
	    pf_info[frame_index].frame_index = frame_index;
    }
  }
  if(kmem.use_lock)
    release(&kmem.lock);
  return (char*)r;
}

