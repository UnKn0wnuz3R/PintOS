#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "threads/vaddr.h"
#include "userprog/process.h"
#include "devices/input.h"
#include "userprog/pagedir.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

void check_valid_addr(void *addr);
static void syscall_handler (struct intr_frame *f);

void check_valid_addr(void *addr){
  if(addr == NULL || !is_user_vaddr(addr) || (unsigned int)addr < 0x08048000) exit(-1); 
  //padedir_get_page 가 false면
  struct thread *t = thread_current();
  if(!pagedir_get_page(t->pagedir,addr)) exit(-1);
   
}
void syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void syscall_handler (struct intr_frame *f) 
{
  int fd;
  char *buf;
  unsigned size;
  pid_t pid;
  void *esp = f->esp;

  check_valid_addr(esp);

  int syscall_num = *(int *)(esp);
  switch(syscall_num){
      case SYS_HALT:
              halt();
              break;
      case SYS_EXIT:
              check_valid_addr(esp+4);
              int status = *(int *)(esp + 4);
              exit(status);
              break;
      case SYS_EXEC:
              // char *cmd_line;
              check_valid_addr(esp+4);
              char *cmd_line = (const char *)*(int *)(esp+4);
              f->eax = exec(cmd_line);
              break;
      case SYS_WAIT:
              check_valid_addr(esp+4);
              pid = (pid_t)*(int *)(esp+4);
              f->eax = wait(pid);
              break;
      case SYS_READ:
              check_valid_addr(esp+4);
              check_valid_addr(esp+8);
              check_valid_addr(esp+12);
              fd = *(int *)(esp+4);
              buf = (const char *)*(int *)(esp+8);
              size = (unsigned)*(int *)(esp+12);
              f->eax = read(fd,buf,size);
              break;
      case SYS_WRITE:
              check_valid_addr(esp+4);
              check_valid_addr(esp+8);
              check_valid_addr(esp+12);
              fd = *(int *)(esp+4);
              buf = (const char *)*(int *)(esp+8);
              size = (unsigned)*(int *)(esp+12);
              f->eax = write(fd,buf,size);
              break;
      case SYS_MAX_FOUR_INT:
              check_valid_addr(esp+4);
              check_valid_addr(esp+8);
              check_valid_addr(esp+12);
              check_valid_addr(esp+16);
              int x = *(int *)(esp+4),
                  y = *(int *)(esp+8),
                  z = *(int *)(esp+12),
                  w = *(int *)(esp+16);
              f->eax = max_of_four_int(x,y,z,w);
              break;
      case SYS_FIBO:
              check_valid_addr(esp+4);
              int n = *(int *)(esp+4); 
              f->eax = fibonacci(n);
              break;
  }
  // printf ("system call!\n");
  // thread_exit ();
}

void halt(void){
  shutdown_power_off();
}

void exit(int status){
  struct thread *t = thread_current();
  t->exit_status = status;
  printf("%s: exit(%d)\n",t->name,status);
  thread_exit();
}

pid_t exec(const char *cmd_line){
  check_valid_addr((void *)cmd_line);
  return process_execute(cmd_line); 
}

int wait(pid_t pid){
  return process_wait(pid);
}

int read(int fd,void *buffer,unsigned size){
  int read_size=0;
  char *read_buf = (char *)buffer;
  if(fd == 0){
    for(int i=0;i<(int)size;i++){
      read_buf[i] = (char)input_getc();
      if(read_buf[i] == '\0') break;
      read_size++;
    }
    return read_size;
  }
  else{
    return -1;
  }
}

int write(int fd, const void *buffer, unsigned size){
  int write_size=0;
  char *write_buf = (char *)buffer;
  if(fd == 1){
    putbuf(write_buf,size);
    return write_size;
  }
  else{
    return -1;
  }
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