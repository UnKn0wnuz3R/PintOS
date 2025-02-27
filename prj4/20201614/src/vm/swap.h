#ifndef VM_SWAP_H
#define VM_SWAP_H

#include "devices/block.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include <bitmap.h>


void swap_init (void);
size_t swap_out (void *frame);
void swap_in (size_t used_index, void *frame);
#endif 