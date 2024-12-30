#ifndef VM_FRAME_H
#define VM_FRAME_H

#include "threads/malloc.h"
#include "threads/palloc.h"
#include "threads/synch.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "filesys/file.h"
#include "vm/page.h"
#include "vm/swap.h"
#include <stdint.h>
#include <stdbool.h>
#include <list.h>

/*
Frame Tbale은 PHY_FRAME의 eviction 구현위해 필요
PT는 각 PHY_FRAME당 하나의 entry, 각 entry는 Page를 가리키는 Pointer 포함(따라서 구조체에 page ptr 필요)
다른건 내가 알아서 추가하고
Frame Table은 PHY_FRAME 부족할때 evict할 page 결정 (도움을 준다)
*/

struct frame_entry{
  void *frame;
  struct vm_entry *vme;
  struct list_elem elem;
  struct thread *thread;
};

void frame_table_init(void);
void free_frame_from_table(void *frame);
static void find_evict_frame(void);
struct frame_entry *frame_alloc(enum palloc_flags flag);

#endif 