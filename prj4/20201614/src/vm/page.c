#include "vm/page.h"

static unsigned vm_hash_func(const struct hash_elem *e, void *aux){
    struct vm_entry *vme = hash_entry(e,struct vm_entry,elem);
    return hash_int((int)(vme->vaddr));
}
static bool vm_less_func(const struct hash_elem *h1,const struct hash_elem *h2,void *aux UNUSED){
    struct vm_entry *vme1 = hash_entry(h1,struct vm_entry,elem);
    struct vm_entry *vme2 = hash_entry(h2,struct vm_entry,elem);
    
    if(vme1->vaddr < vme2->vaddr){
        return true;
    }
    else{
        return false;
    }
}
void vm_init(struct hash *vm){
    hash_init(vm,vm_hash_func,vm_less_func,NULL);
}
bool insert_vme(struct hash *vm, struct vm_entry *vme){
    if(!hash_insert(vm,&(vme->elem))) return false;
    else return true;
}
bool delete_vme(struct hash *vm, struct vm_entry *vme){
    if(hash_delete(vm,&(vme->elem)) != NULL){
        return true;
    } 
    else{
        return false;
    } 
}
struct vm_entry *find_vme(void *vaddr){
    struct vm_entry vme;
    vme.vaddr = pg_round_down(vaddr);

    struct thread *curr_thread = thread_current();
    struct hash_elem *hash_ptr = hash_find(&(curr_thread->vm),&(vme.elem));
    
    if(hash_ptr != NULL){
        struct vm_entry *vme_ptr = hash_entry(hash_ptr,struct vm_entry,elem);
        return vme_ptr;
    }
    
    return NULL;
}
void vm_destructor(struct hash_elem *e,void *aux){
    struct thread *curr_thread = thread_current();
    struct vm_entry *vme = hash_entry(e,struct vm_entry,elem);
    
    free_frame_from_table(pagedir_get_page(curr_thread->pagedir,vme->vaddr));
    pagedir_clear_page(thread_current()->pagedir,vme->vaddr);
    free(vme);
}
void vm_destroy(struct hash *vm){
    hash_destroy(vm,vm_destructor);
}

