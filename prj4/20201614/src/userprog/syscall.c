#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stdbool.h>
#include <string.h>
#include "devices/timer.h"
#include <filesys/file.h>
#include <filesys/filesys.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "userprog/pagedir.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "vm/mmap.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
struct lock file_lock;
static void syscall_handler (struct intr_frame *f);

struct vm_entry *check_valid_addr(void *addr,void *esp){
  // bool load = false;

  if(addr == NULL || !is_user_vaddr(addr)) exit(-1); 
  //padedir_get_page 가 false면
  // struct thread *t = thread_current();
  // if(!pagedir_get_page(t->pagedir,addr)) exit(-1);
  struct vm_entry *vme = find_vme(addr);
  if(!vme){
    if(!grow_stack(addr,esp))  exit(-1);
  }
  else return vme;
}
/* read() syscall의 buf가 valid VA인지 확인 */
void check_valid_buffer(void *buf, unsigned size,void *esp,bool to_write){
  char *temp_buf = (char *)buf;
  for(int i=0;i<size;i++){
    struct vm_entry *vme = check_valid_addr(temp_buf,esp);
    
    if((vme != NULL) && to_write && !vme->writable){
      exit(-1);
    }

    temp_buf++;
  }
}
/* write() syscall 시 사용 */
void check_valid_string(const void *str, void *esp){
  check_valid_addr(str,esp);

  for(;*(char *)str != 0;str = (char *)str + 1){
    check_valid_addr(str,esp);
  }

}
void syscall_init (void) 
{
  lock_init(&file_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler (struct intr_frame *f) 
{
  int fd;
  char *buf, *file;
  unsigned size,position;
  pid_t pid;
  void *esp = f->esp;

  check_valid_addr(esp,esp);
  int syscall_num = *(int *)(esp);
  switch(syscall_num){
      case SYS_HALT:
              halt();
              break;
      case SYS_EXIT:
              check_valid_addr((uint8_t*)esp+4,esp);
              int status = *(int *)((uint8_t*)esp + 4);
              exit(status);
              break;
      case SYS_EXEC:
              // char *cmd_line;
              check_valid_addr((uint8_t*)esp+4,esp);
              check_valid_string((const void *)(esp+4),esp);
              char *cmd_line = *(char **)((uint8_t*)esp+4);
              f->eax = exec(cmd_line);
              break;
      case SYS_WAIT:
              check_valid_addr((uint8_t*)esp+4,esp);
              pid = *(pid_t *)((uint8_t*)esp+4);
              f->eax = wait(pid);
              break;
      case SYS_READ:
              check_valid_addr((uint8_t*)esp+4,esp);
              check_valid_addr((uint8_t*)esp+8,esp);
              check_valid_addr((uint8_t*)esp+12,esp);
              fd = *(int *)((uint8_t*)esp+4);
              buf = *(void **)((uint8_t*)esp+8);
              size = *(unsigned *)((uint8_t*)esp+12);
              check_valid_buffer(buf,size,esp,true);
              f->eax = read(fd,buf,size);
              break;
      case SYS_WRITE:
              check_valid_addr((uint8_t*)esp+4,esp);
              check_valid_addr((uint8_t*)esp+8,esp);
              check_valid_addr((uint8_t*)esp+12,esp);
              fd = *(int *)((uint8_t*)esp+4);
              buf = *(void **)((uint8_t*)esp+8);
              size = *(unsigned *)((uint8_t*)esp+12);
              check_valid_buffer(buf,size,esp,false);
              f->eax = write(fd,buf,size);
              break;
      case SYS_MAX_FOUR_INT:
              check_valid_addr(esp+4,esp);
              check_valid_addr(esp+8,esp);
              check_valid_addr(esp+12,esp);
              check_valid_addr(esp+16,esp);
              int x = *(int *)(esp+4),
                  y = *(int *)(esp+8),
                  z = *(int *)(esp+12),
                  w = *(int *)(esp+16);
              f->eax = max_of_four_int(x,y,z,w);
              break;
      case SYS_FIBO:
              check_valid_addr(esp+4,esp);
              int n = *(int *)(esp+4);
              f->eax = fibonacci(n);
              break;
     case SYS_CREATE:
              check_valid_addr((uint8_t*)esp+4,esp);
              check_valid_addr((uint8_t*)esp+8,esp);
              check_valid_string((const void *)esp+4,esp);
              file = *(char **)((uint8_t*)esp+4); 
              size = *(unsigned *)((uint8_t*)esp+8);
              f->eax = create(file,size);
              break;
      case SYS_REMOVE:
              check_valid_addr((uint8_t*)esp+4,esp);
              check_valid_string((const void *)esp+4,esp);
              file = *(char **)((uint8_t*)esp+4); 
              f->eax = remove(file);
              break;
      case SYS_OPEN:
              check_valid_addr((uint8_t*)esp+4,esp);
              check_valid_string((const void *)esp+4,esp);
              file = *(char **)((uint8_t*)esp+4);
              f->eax = open(file);
              break;
      case SYS_FILESIZE:
              check_valid_addr((uint8_t*)esp+4,esp);
              fd = *(int *)((uint8_t*)esp+4);
              f->eax = filesize(fd);
              break;
      case SYS_SEEK:
              check_valid_addr((uint8_t*)esp+4,esp);
              check_valid_addr((uint8_t*)esp+8,esp);
              fd = *(int *)((uint8_t*)esp+4); 
              position = *(unsigned *)((uint8_t*)esp+8);
              seek(fd,position);
              break;
      case SYS_TELL:
              check_valid_addr((uint8_t*)esp+4,esp);
              fd = *(int *)((uint8_t*)esp+4);
              f->eax = tell(fd);
              break;
      case SYS_CLOSE:
              check_valid_addr((uint8_t*)esp+4,esp);
              fd = *(int *)((uint8_t*)esp+4);
              close(fd);
              break;
  }
}

void halt(void){
  shutdown_power_off();
}

void exit(int status){

  thread_current()->exit_status = status;
  printf("%s: exit(%d)\n",thread_current()->name,status);
  
  for(int i=2;i<128;i++){
    if(thread_current()->fd_set[i] != NULL){
      close(i);
    }
  }
	
  struct list_elem *elem_ptr = list_begin(&(thread_current()->child_list));
  while(elem_ptr != list_end(&(thread_current()->child_list))){
	  struct thread *child = list_entry(elem_ptr,struct thread,child_elem);
	  wait(child->tid);
	  elem_ptr = list_next(elem_ptr);
  }

  thread_exit();
}

pid_t exec(const char *cmd_line){
  return process_execute(cmd_line); 
}

int wait(pid_t pid){
  return process_wait(pid);
}

int fibonacci(int n){
  int a1=1, a2=1, ans=0;
  
  if(n < 0){
    printf("\"N\" must be positive integer!\n");
    exit(-1);
  }
  else if(n<=2) return 1;
  else{
    for(int i=2;i<n;i++){
      ans = a1+a2;
      a1 = a2;
      a2 = ans;
    }
    return ans;
  }
}
int max_of_four_int(int x,int y,int z,int w){
  int ans;

  ans = MAX(x,y);
  ans = MAX(ans,z);
  ans = MAX(ans,w);

  return ans;
}

bool create(const char *file, unsigned initial_size){
  if(file == NULL || !is_user_vaddr(file)) exit(-1); 
  lock_acquire(&file_lock);
  bool flag = filesys_create(file,initial_size);
  lock_release(&file_lock);
  if(flag) return true;
  else return false;
}
bool remove(const char *file){
  if(file == NULL || !is_user_vaddr(file)) exit(-1); 
  lock_acquire(&file_lock);
  bool flag = filesys_remove(file);
  lock_release(&file_lock);
  if(flag) return true;
  else return false;
}
int read(int fd,void *buffer,unsigned size){
  if(buffer == NULL || !is_user_vaddr(buffer)) exit(-1); 

  int read_size=0;
  char *read_buf = (char *)buffer;
  lock_acquire(&file_lock);
  if(fd == 0){
    for(int i=0;i<(int)size;i++){
      read_buf[i] = (char)input_getc();
      if(read_buf[i] == '\0') break;
      read_size++;
    }
  }
  else{
    if(thread_current()->fd_set[fd] != NULL){
      read_size = file_read(thread_current()->fd_set[fd],buffer,size);
    }
    else{
      lock_release(&file_lock);
      exit(-1);
    }
  }
  lock_release(&file_lock);
  return read_size;
}

int write(int fd, const void *buffer, unsigned size){
  if(buffer == NULL || !is_user_vaddr(buffer)) exit(-1); 
  
  int write_size=0;
  char *write_buf = (char *)buffer;
  lock_acquire(&file_lock);
  if(fd == 1){
    putbuf(write_buf,size);
    write_size = size;
  }
  else{
    if(fd < 2 || fd > 127){
      lock_release(&file_lock);
      exit(-1);
    }
    else{
      if(thread_current()->fd_set[fd] != NULL){
        write_size = file_write(thread_current()->fd_set[fd],buffer,size);
      }
    }
  }
  lock_release(&file_lock);
  return write_size;
}
/*Opens the file called file. Returns a nonnegative integer handle called a “file descriptor” (fd), or -1 if the file could not be opened.
File descriptors numbered 0 and 1 are reserved for the console: fd 0 (STDIN_FILENO) is
standard input, fd 1 (STDOUT_FILENO) is standard output. The open system call will
never return either of these file descriptors, which are valid as system call arguments
only as explicitly described below.
Each process has an independent set of file descriptors. File descriptors are not
inherited by child processes.
When a single file is opened more than once, whether by a single process or different
processes, each open returns a new file descriptor. Different file descriptors for a single
file are closed independently in separate calls to close and they do not share a file
position*/
int open (const char *file){
  int fd;
  if(file == NULL || !is_user_vaddr(file)) exit(-1); 

  lock_acquire(&file_lock);

  struct file *f = filesys_open(file);
  if(f == NULL){
    lock_release(&file_lock);
    return -1;
  }

  if(strcmp(thread_current()->name,file) == 0){
    file_deny_write(f);
  }
  fd = thread_current()->next_fd;
  thread_current()->fd_set[thread_current()->next_fd] = f;
  thread_current()->next_fd = thread_current()->next_fd+1;
  lock_release(&file_lock);
  return fd;
}
// Returns the size, in bytes, of the file open as fd.
int filesize (int fd){
  int file_size = 0;
  if(fd < 0 || fd > 127 || (thread_current()->fd_set[fd] == NULL)){
    exit(-1);
  }
  else{
    file_size = file_length(thread_current()->fd_set[fd]); 
  }
  return file_size;
}
/*Changes the next byte to be read or written in open file fd to position, expressed in
bytes from the beginning of the file. (Thus, a position of 0 is the file’s start.)
A seek past the current end of a file is not an error. A later read obtains 0 bytes,
indicating end of file. A later write extends the file, filling any unwritten gap with
zeros. (However, in Pintos files have a fixed length until project 4 is complete, so
writes past end of file will return an error.) These semantics are implemented in the
file system and do not require any special effort in system call implementation.*/
void seek (int fd, unsigned position){
  if(fd < 0 || fd > 127 || (thread_current()->fd_set[fd] == NULL)){
    exit(-1);
  }
  else{
    file_seek(thread_current()->fd_set[fd],position);
  }
}
/*Returns the position of the next byte to be read or written in open file fd, expressed
in bytes from the beginning of the file.*/
unsigned tell (int fd){
  unsigned offset = 0;
  if(fd < 0 || fd > 127 || (thread_current()->fd_set[fd] == NULL)){
    exit(-1);
  }
  else{
    offset = file_tell(thread_current()->fd_set[fd]);
  }
  return offset;
}
void close (int fd){
  lock_acquire(&file_lock);
  if(fd < 0 || fd > 127 || (thread_current()->fd_set[fd] == NULL)){
    lock_release(&file_lock);
    exit(-1);
  }
  file_close(thread_current()->fd_set[fd]);
  thread_current()->fd_set[fd] = NULL;
  lock_release(&file_lock);
}
