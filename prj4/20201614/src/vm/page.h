#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "threads/thread.h"
#include "threads/vaddr.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include "lib/kernel/hash.h"
#include "lib/debug.h"
#include "filesys/file.h"
#include "filesys/filesys.h"
#include "filesys/off_t.h"
#include "userprog/pagedir.h"
#include "userprog/syscall.h"
#include "userprog/process.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "vm/frame.h"
#include "vm/swap.h"

#define MAX_STACK_SIZE (1 << 23)

#define VM_BIN 0        // load data from binary file
#define VM_FILE 1       // load data from memory mapped file
#define VM_ANON 2       // load data from swap space 

/*
SPT는 page fualt를 다루기 위함.
기존 PT의 한계로 인해 SPT가 필요 
K는 PF를 일으킨 VP의 정보를 찾기위해 SPT에서 확인.
K는 Process 종료시에 어떤 자원 해제할지 결정위해 SPT 필요
SPT의 entry들을 추적하기위해 SPT의 PT를 사용하기도 함 (thread->vm 변수)

PintOS는 현재 2-level PT 구조 사용(thread->pagedir)

메모리에 접근할 경우, 해당 주소의 VP를 표현하는 vm_entry를 탐색해야함.
따라서 hash.c 잘 살펴보자

hash_destroy() -> hash table을 삭제
hash_delete() -> hash table에서 element를 제거

Hash Table을 이용해서 구현해야할 부분
thread 구조체에 hash_table 자료구조 추가
process 생성시 -> hash_table 초기화, vm_entry들 hash_table에 추가
process 실행중 -> PF발생 시, vm_entry를 hash_table에서 탐색
process 종료시 -> hash_table의 bucket_list와 vm_entry들 제거 
*/
struct vm_entry{
    uint8_t type;       // VM_BIN, VM_FILE, VM_ANON
    void *vaddr;        // vm_entry가 관리하는 VP "number" -> 이걸로 hash 값 추출
    bool writable;      // true -> 해당 주소에 write allowed
    bool is_loaded;     // is it in memory?
    
    struct file *file;  // ref file ptr (mapped with VA)
    off_t offset;       // ref file offset(memory mapped file)
    size_t bytes_read;  // amount of data int the page
    size_t bytes_zero;  // amount of data int the page

    struct hash_elem elem;
    
    size_t swap_idx;    // location in the swap area
};

void vm_init(struct hash *vm);
static unsigned vm_hash_func(const struct hash_elem *e, void *aux);
bool insert_vme(struct hash *vm, struct vm_entry *vme);
bool delete_vme(struct hash *vm, struct vm_entry *vme);
struct vm_entry *find_vme(void *vaddr);
void vm_destructor(struct hash_elem *e,void *aux);
void vm_destroy(struct hash *vm);

#endif /* vm/page.h */