#include "vm/swap.h"

#define SECTORS_PER_PAGE (PGSIZE / BLOCK_SECTOR_SIZE)

struct bitmap *swap_map;
struct lock swap_lock;
struct block *swap_block;

void swap_init(void){
    swap_map = bitmap_create(PGSIZE);
    if(!swap_map)   return;
    bitmap_set_all(swap_map,0);
    lock_init(&swap_lock);
}
void swap_in(size_t used_index,void *frame){
    int max_offset = 8;

    lock_acquire(&swap_lock);
    
    if(used_index > 0){
        swap_block = block_get_role(BLOCK_SWAP);
        used_index-=1;
        
        for(size_t i=0;i<max_offset;i++){
            block_read(swap_block,used_index*max_offset + i,frame + i*BLOCK_SECTOR_SIZE);
            bitmap_set(swap_map,used_index,false);
        }
    }
    lock_release(&swap_lock);
}
size_t swap_out(void *frame){
    int max_offset = 8;
    
    lock_acquire(&swap_lock);
    
    swap_block = block_get_role(BLOCK_SWAP);
    size_t free_idx = bitmap_scan_and_flip(swap_map,0,1,false);
    
    for(size_t i=0;i<max_offset;i++){
        block_write(swap_block,free_idx*max_offset + i,frame + i*BLOCK_SECTOR_SIZE);
    }
    
    free_idx += 1;
    
    lock_release(&swap_lock);
    
    return free_idx;
}
