#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include <stdbool.h>
#include "userprog/pagedir.h"
#include "vm/page.h"
#include "vm/frame.h"
#include "vm/swap.h"
#include "vm/mmap.h"

extern struct lock file_lock;

#define bool	_Bool
typedef int pid_t;

struct vm_entry *check_valid_addr(void *addr,void *esp);
void check_valid_buffer(void *buf, unsigned size,void *esp,bool to_write);
void check_valid_string(const void *str, void *esp);
void syscall_init (void);
void halt(void);
void exit(int status);
pid_t exec(const char *cmd_line);
int wait(pid_t pid);
int read(int fd,void *buffer,unsigned size);
int write(int fd, const void *buffer, unsigned size);
int fibonacci(int n);
int max_of_four_int(int x,int y,int z,int w);
bool create(const char *file, unsigned initial_size);
bool remove(const char *file);
int open (const char *file);
int filesize (int fd);
void seek (int fd, unsigned position);
unsigned tell (int fd);
void close (int fd);

#endif /* userprog/syscall.h */
