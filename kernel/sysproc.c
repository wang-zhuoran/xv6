#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "sysinfo.h"

struct kmem;
void freemem(uint64 *freemem);
void nproc(uint64* n);
// struct sysinfo;

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64 sys_trace(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;

  myproc()->trace_mask = n;

  return 0;
}

uint64 sys_sysinfo(void)
{
  struct sysinfo info;
  // 首先把用户空间传入的sysinfo struct 读入到内核空间
  uint64 addr;

  if(argaddr(0, (uint64*)&addr) < 0)
    return -1;

  // 然后将内核空间的sysinfo struct填充
  freemem(&info.freemem);
  nproc(&info.nproc);

  // copyout 内核空间的sysinfo struct 到用户空间
  if(copyout(myproc()->pagetable, addr, (char*)&info, sizeof info) < 0)
    return -1;

  return 0;
}

