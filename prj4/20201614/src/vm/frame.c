#include "vm/frame.h"

struct lock frame_table_lock;
struct list ft_list;
struct list_elem *ft_clock;

void frame_table_init(void){
    ft_clock = NULL;
    list_init(&ft_list);
    lock_init(&frame_table_lock);
}
struct frame_entry *frame_alloc(enum palloc_flags flag){
    
    struct frame_entry *frame_ptr = (struct frame_entry*)malloc(sizeof(struct frame_entry));
    
    if(!frame_ptr) return NULL;
    else{
        memset(frame_ptr,0,sizeof(struct frame_entry));    
        frame_ptr->frame = palloc_get_page(flag);
        if(frame_ptr->frame){
            frame_ptr->thread = thread_current();
            lock_acquire(&frame_table_lock);
            list_push_back(&ft_list,&(frame_ptr->elem));
            lock_release(&frame_table_lock);
        } 
        else{
            while(!frame_ptr->frame){
                lock_acquire(&frame_table_lock);
                find_evict_frame();
                lock_release(&frame_table_lock);
                frame_ptr->frame = palloc_get_page(flag);
            }
            frame_ptr->thread = thread_current();
            lock_acquire(&frame_table_lock);
            list_push_back(&ft_list,&(frame_ptr->elem));
            lock_release(&frame_table_lock);
        }
    }
    
    return frame_ptr;
}
void free_frame_from_table(void *frame){

    lock_acquire(&frame_table_lock);
    struct list_elem *res;
    struct list_elem *elem_ptr = list_begin(&ft_list);
    
    while(elem_ptr != list_end(&ft_list)){
        struct frame_entry *frame_ptr = list_entry(elem_ptr,struct frame_entry,elem);
        if(frame_ptr->frame == frame){
            struct list_elem *curr_elem = &(frame_ptr->elem);
            struct list_elem *next_elem = list_remove(&(frame_ptr->elem));

            if(curr_elem == ft_clock)    ft_clock = next_elem;

            pagedir_clear_page(frame_ptr->thread->pagedir,frame_ptr->vme->vaddr);
            palloc_free_page(frame_ptr->frame);
            free(frame_ptr);
            break;
        }
        elem_ptr = list_next(elem_ptr);
    }

    lock_release(&frame_table_lock);
}
static struct list_elem *ft_clock_advance(void)
{
  if (ft_clock == NULL || ft_clock == list_end(&ft_list)) {
    if (list_empty(&ft_list) == false){
        ft_clock = list_begin(&ft_list);
    }
    return ft_clock;
  }

  struct list_elem *next_elem = list_next(ft_clock);
  if (next_elem == list_end(&ft_list))
    next_elem = list_begin(&ft_list);

  ft_clock = next_elem;
  return ft_clock;
}

static void find_evict_frame(void) {
    struct frame_entry *frame_ptr=NULL;
    struct list_elem *elem_ptr;
    while(true){
      elem_ptr = ft_clock_advance();
      frame_ptr = list_entry(elem_ptr,struct frame_entry, elem);

      if(!pagedir_is_accessed(frame_ptr->thread->pagedir, frame_ptr->vme->vaddr)){
        break;
      }
      
      pagedir_set_accessed(frame_ptr->thread->pagedir, frame_ptr->vme->vaddr, 0);
    }

    if(!frame_ptr)  return;
    bool is_dirty = pagedir_is_dirty(frame_ptr->thread->pagedir, frame_ptr->vme->vaddr);

    switch(frame_ptr->vme->type){
      case VM_BIN:
          if(is_dirty){
            frame_ptr->vme->type = VM_ANON;
            frame_ptr->vme->swap_idx = swap_out (frame_ptr->frame);
          } 
          break;
      case VM_FILE:
          if(is_dirty){
            file_write_at(frame_ptr->vme->file, frame_ptr->frame, frame_ptr->vme->bytes_read, frame_ptr->vme->offset);
          }
          break;
      case VM_ANON:
          frame_ptr->vme->swap_idx = swap_out(frame_ptr->frame);
          break;
      default: 
          break;
    }

    frame_ptr->vme->is_loaded = false;
    // evict
    struct list_elem *curr_elem = &(frame_ptr->elem);
    struct list_elem *next_elem = list_remove(&(frame_ptr->elem));
    if(curr_elem == ft_clock)    ft_clock = next_elem;
    
    pagedir_clear_page(frame_ptr->thread->pagedir, frame_ptr->vme->vaddr);
    palloc_free_page(frame_ptr->frame);
    free(frame_ptr);

}



