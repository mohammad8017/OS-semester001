#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "x86.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct proc proc[NPROC];
} ptable;

static struct proc *initproc;

int nextpid = 1;
// int nexttid = 1;
extern void forkret(void);
extern void trapret(void);

static void wakeup1(void *chan);

void
pinit(void)
{
  initlock(&ptable.lock, "ptable");
}

// Must be called with interrupts disabled
int
cpuid() {
  return mycpu()-cpus;
}

// Must be called with interrupts disabled to avoid the caller being
// rescheduled between reading lapicid and running through the loop.
struct cpu*
mycpu(void)
{
  int apicid, i;
  
  if(readeflags()&FL_IF)
    panic("mycpu called with interrupts enabled\n");
  
  apicid = lapicid();
  // APIC IDs are not guaranteed to be contiguous. Maybe we should have
  // a reverse map, or reserve a register to store &cpus[i].
  for (i = 0; i < ncpu; ++i) {
    if (cpus[i].apicid == apicid)
      return &cpus[i];
  }
  panic("unknown apicid\n");
}

// Disable interrupts so that we are not rescheduled
// while reading proc from the cpu structure
struct proc*
myproc(void) {
  struct cpu *c;
  struct proc *p;
  pushcli();
  c = mycpu();
  p = c->proc;
  popcli();
  return p;
}

//PAGEBREAK: 32
// Look in the process table for an UNUSED proc.
// If found, change state to EMBRYO and initialize
// state required to run in the kernel.
// Otherwise return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;
  char *sp;

  acquire(&ptable.lock);

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == UNUSED)
      goto found;

  release(&ptable.lock);
  return 0;

found:
  p->state = EMBRYO;
  p->pid = nextpid++;

  release(&ptable.lock);

  // Allocate kernel stack.
  if((p->kstack = kalloc()) == 0){
    p->state = UNUSED;
    return 0;
  }
  sp = p->kstack + KSTACKSIZE;

  // Leave room for trap frame.
  sp -= sizeof *p->tf;
  p->tf = (struct trapframe*)sp;

  // Set up new context to start executing at forkret,
  // which returns to trapret.
  sp -= 4;
  *(uint*)sp = (uint)trapret;

  sp -= sizeof *p->context;
  p->context = (struct context*)sp;
  memset(p->context, 0, sizeof *p->context);
  p->context->eip = (uint)forkret;

  return p;
}

//PAGEBREAK: 32
// Set up first user process.
void
userinit(void)
{
  struct proc *p;
  extern char _binary_initcode_start[], _binary_initcode_size[];

  p = allocproc();
  
  initproc = p;
  if((p->pgdir = setupkvm()) == 0)
    panic("userinit: out of memory?");
  inituvm(p->pgdir, _binary_initcode_start, (int)_binary_initcode_size);
  p->sz = PGSIZE;
  memset(p->tf, 0, sizeof(*p->tf));
  p->tf->cs = (SEG_UCODE << 3) | DPL_USER;
  p->tf->ds = (SEG_UDATA << 3) | DPL_USER;
  p->tf->es = p->tf->ds;
  p->tf->ss = p->tf->ds;
  p->tf->eflags = FL_IF;
  p->tf->esp = PGSIZE;
  p->tf->eip = 0;  // beginning of initcode.S

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  // this assignment to p->state lets other cores
  // run this process. the acquire forces the above
  // writes to be visible, and the lock is also needed
  // because the assignment might not be atomic.
  acquire(&ptable.lock);

  p->state = RUNNABLE;

  release(&ptable.lock);
}

// Grow current process's memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *curproc = myproc();

  sz = curproc->sz;
  if(n > 0){
    if((sz = allocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  } else if(n < 0){
    if((sz = deallocuvm(curproc->pgdir, sz, sz + n)) == 0)
      return -1;
  }
  curproc->sz = sz;
  switchuvm(curproc);
  return 0;
}

// Create a new process copying p as the parent.
// Sets up stack to return as if from system call.
// Caller must set state of returned proc to RUNNABLE.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *curproc = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy process state from proc.
  if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
    kfree(np->kstack);
    np->kstack = 0;
    np->state = UNUSED;
    return -1;
  }
  np->sz = curproc->sz;
  np->parent = curproc;
  *np->tf = *curproc->tf;
  // For threads
  np->isthread = 0;
  np->tid = np->pid;

  // Clear %eax so that fork returns 0 in the child.
  np->tf->eax = 0;

  for(i = 0; i < NOFILE; i++)
    if(curproc->ofile[i])
      np->ofile[i] = filedup(curproc->ofile[i]);
  np->cwd = idup(curproc->cwd);

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  pid = np->pid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;

  release(&ptable.lock);

  return pid;
}

// Clone system call to create threads
int
clone(void (*func_ptr)(void *), void *stack, int flags, void *arg) 
{
  int i, tid;
  struct proc *np;
  struct proc *curproc = myproc();
  uint ustack[2], sp;
  // int ret;
  // int CLONE_FILES, CLONE_FS, CLONE_PARENT, CLONE_VM, CLONE_THREAD;
  if(flags < 0 || flags > 31) {
    return -1;
  }
  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Checking the flags
  if (flags & (1 << (1 - 1)))
    np->CLONE_FILES = 1;
  else
    np->CLONE_FILES = 0;
  if (flags & (1 << (2 - 1)))
    np->CLONE_FS = 1;
  else
    np->CLONE_FS = 0;
  if (flags & (1 << (3 - 1)))
    np->CLONE_VM = 1;
  else
    np->CLONE_VM = 0;
  if (flags & (1 << (4 - 1)))
    np->CLONE_THREAD = 1;
  else
    np->CLONE_THREAD = 0;
  if (flags & (1 << (5 - 1)))
    np->CLONE_PARENT = 1;
  else
    np->CLONE_PARENT = 0;
  

  if(np->CLONE_PARENT == 1) {
    if(curproc->isthread == 0) {
      np->parent = curproc->parent;
    } else {
      np->parent = (curproc->parent)->parent;
    }
  } else {
    if(curproc->isthread == 0) {
      np->parent = curproc;
    } else {
      np->parent = curproc->parent;
    }
  }
  np->threadparent = curproc;
  // if(curproc->isthread == 0) {
  //   curproc->threadcount += 1;
  //   np->parent = curproc;
  // } else {
  //   (curproc->threadparent)->threadcount += 1;
  //   np->parent = curproc->parent;
  // }

  // np->tid = nexttid++;
  if(np->CLONE_THREAD == 1) {
    np->tid = np->pid;
    np->pid = curproc->pid;
    // np->tgid = curproc->tgid;
  } else {
    np->tid = np->pid;
    // np->tgid = np->pid;
  }

  if(np->CLONE_VM == 1) {
    // Copy process state from proc.
    // if((np->pgdir = copyuvm_clone(curproc->pgdir, curproc->sz)) == 0) {
    //   kfree(np->kstack);
    //   np->kstack = 0;
    //   np->state = UNUSED;
    //   return -1;
    // }
    np->pgdir = curproc->pgdir;
  } else {
    // Copy process state from proc.
    if((np->pgdir = copyuvm(curproc->pgdir, curproc->sz)) == 0){
      kfree(np->kstack);
      np->kstack = 0;
      np->state = UNUSED;
      return -1;
    }
  }
  
  // np->pid = curproc->pid;
  np->sz = curproc->sz;
  *np->tf = *curproc->tf;
  np->isthread = 1;
  np->ustack = (uint)stack;

  // Clear %eax so that clone returns 0 in the child.
  np->tf->eax = 0;
  np->tf->eip = (uint)func_ptr;
  sp = (uint)stack + PGSIZE;
  np->tf->ebp = sp;
  np->tf->esp = sp;

  ustack[0] = 0xffffffff;
  ustack[1] = (uint)arg;
  
  np->tf->esp = np->tf->esp - 8;
  if(copyout(np->pgdir, np->tf->esp, ustack, 8) < 0) {
    return -1;
  }

  if(np->CLONE_FILES == 1) {
    for(i = 0; i < NPROC; i++) {
      if(np->ofileshare[i] == 0) {
        np->ofileshare[i] = curproc;
      }
    }
    for(i = 0; i < NPROC; i++) {
      if(curproc->ofileshare[i] == 0) {
        curproc->ofileshare[i] = np;
      }
    }
    for(i = 0; i < NOFILE; i++)
      if(curproc->ofile[i])
        np->ofile[i] = curproc->ofile[i];
  } else {
    for(i = 0; i < NOFILE; i++)
      if(curproc->ofile[i])
        np->ofile[i] = filedup(curproc->ofile[i]);
  }

  // for(i = 0; i < NOFILE; i++)
  //   if(curproc->ofile[i])
  //     np->ofile[i] = filedup(curproc->ofile[i]);

  if(np->CLONE_FS == 1) {
    for(i = 0; i < NPROC; i++) {
      if(np->cwdshare[i] == 0) {
        np->cwdshare[i] = curproc;
      }
    }
    for(i = 0; i < NPROC; i++) {
      if(curproc->cwdshare[i] == 0) {
        curproc->cwdshare[i] = np;
      }
    }
    np->cwd = curproc->cwd;
  } else {
    np->cwd = idup(curproc->cwd);
  }

  safestrcpy(np->name, curproc->name, sizeof(curproc->name));

  tid = np->tid;

  acquire(&ptable.lock);

  np->state = RUNNABLE;
  
  release(&ptable.lock);

  return tid;
}

// Join system call
int
join(int tid)
{
  // int a[1] = {5};
  // *stack = (void*)(a);
  // cprintf("This is join : %d\n", ((int*)(*stack))[0]);
  // return 0;

  struct proc *p, *found;
  int havekids;
  struct proc *curproc = myproc();
  // cprintf("Cur proc id : %d\n", curproc->pid);
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children threads.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      // if p is process
      if(p->tid != tid || p->isthread == 0) 
        continue;
      // check if curproc is thread
      if(p->tid == tid) {
        if(curproc->isthread == 1) {
          found = p;
          // check if p belongs to same thread group
          if(p->pid != curproc->pid && p->parent != curproc && p->threadparent != curproc) {
    // cprintf("Hey ron\n");
            release(&ptable.lock);
            return -1;
          }
        } else {
          // check if p thread have curproc as parent
          if(p->parent != curproc) {
            release(&ptable.lock);
            return -1;
          }
        }
      }

      havekids = 1;
      // cprintf("Join : thread id : %d\n", p->pid);
      if(p->state == ZOMBIE){
        // Found one.
        // cprintf("Found zombie thread ID : %d\n", p->pid);
        tid = p->tid;
        kfree(p->kstack);
        p->kstack = 0;
        if(p->CLONE_VM == 0) {
          freevm(p->pgdir);
        }
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return tid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    curproc->joinproc = found;

    // cprintf("Sleeping\n");
    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

// kills the given thread with given tid here tid and pid are same
int
tkill(int tid)
{
  struct proc *p;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->tid == tid){
      if(p->isthread == 1) {
        p->killed = 1;
        // Wake process from sleep if necessary.
        if(p->state == SLEEPING)
          p->state = RUNNABLE;
        release(&ptable.lock);
        return 0;
      } 
    }
  }
  release(&ptable.lock);
  return -1;
}



// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait() to find out it exited.
void
exit(void)
{
  struct proc *curproc = myproc();
  struct proc *p;
  int fd, i, j, isfileshare = 0, iscwdshare = 0, nojoin = 0;

  if(curproc == initproc)
    panic("init exiting");

  // handle ofile share info
  for(i = 0; i < NPROC; i++) {
    if(curproc->ofileshare[i] != 0) {
      isfileshare = 1;
      p = curproc->ofileshare[i];
      for(j = 0; j < NPROC; j++) {
        if(p->ofileshare[j] == curproc) {
          p->ofileshare[j] = 0;
          break;
        }
      }
      curproc->ofileshare[i] = 0;
    }
  }

  // handle cwd share info
  for(i = 0; i < NPROC; i++) {
    if(curproc->cwdshare[i] != 0) {
      iscwdshare = 1;
      p = curproc->cwdshare[i];
      for(j = 0; j < NPROC; j++) {
        if(p->cwdshare[j] == curproc) {
          p->cwdshare[j] = 0;
          break;
        }
      }
      curproc->cwdshare[i] = 0;
    }
  }

  // Close all open files.
  if(isfileshare == 0) {  
    for(fd = 0; fd < NOFILE; fd++){
      if(curproc->ofile[fd]){
        fileclose(curproc->ofile[fd]);
        curproc->ofile[fd] = 0;
      }
    }
  } else {
    for(fd = 0; fd < NOFILE; fd++){
      if(curproc->ofile[fd]){
        // fileclose(curproc->ofile[fd]);
        curproc->ofile[fd] = 0;
      }
    }
  }

  if(iscwdshare == 0) {
    begin_op();
    iput(curproc->cwd);
    end_op();
  }
  curproc->cwd = 0;

  acquire(&ptable.lock);

  // Parent might be sleeping in wait().
  // wakeup1(curproc->parent);

  // Pass abandoned children to init.
  if(curproc->isthread == 0) {
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent == curproc && p->isthread != 1){
        p->parent = initproc;
        if(p->state == ZOMBIE)
          wakeup1(initproc);
      }
    }
  // }
  
  // if(curproc->isthread == 0) {
    wakeup1(curproc->parent);
    // cprintf("Helo\n");
    // wakeup1(curproc->parent);
  // } else {
    // kill all children of the process
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->isthread == 1 && p->parent == curproc){
        p->killed = 1;
        // Wake process from sleep if necessary.
        if(p->state == SLEEPING)
          p->state = RUNNABLE;
      }
    }
    // tkill(curproc->tid);
    // cprintf("hey ron exit kill child : %d\n", don);
  } else {
    nojoin = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->joinproc == curproc){
        nojoin = 1;
        p->joinproc = 0;
        wakeup1(p);
      }
    }
    if(nojoin == 0) {
      wakeup1(curproc->parent);
    }
  }

  // Jump into the scheduler, never to return.
  curproc->state = ZOMBIE;
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(void)
{
  struct proc *p;
  int havekids, pid;
  struct proc *curproc = myproc();
  
  acquire(&ptable.lock);
  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->parent != curproc)
        continue;
      if(p->isthread == 1) 
        continue;
      // check if curproc is thread and p belongs to same thread group
      if(curproc->isthread == 1) {
        if(p->parent->pid != curproc->pid) 
          continue;
      }
      havekids = 1;
      if(p->state == ZOMBIE){
        // Found one.
        pid = p->pid;
        kfree(p->kstack);
        p->kstack = 0;
        freevm(p->pgdir);
        p->pid = 0;
        p->parent = 0;
        p->name[0] = 0;
        p->killed = 0;
        p->isthread = 0;
        p->tid = 0;
        p->ustack = 0;
        p->state = UNUSED;
        release(&ptable.lock);
        return pid;
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || curproc->killed){
      release(&ptable.lock);
      return -1;
    }

    // Wait for children to exit.  (See wakeup1 call in proc_exit.)
    sleep(curproc, &ptable.lock);  //DOC: wait-sleep
  }
}

//PAGEBREAK: 42
// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run
//  - swtch to start running that process
//  - eventually that process transfers control
//      via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();
  c->proc = 0;
  
  for(;;){
    // Enable interrupts on this processor.
    sti();

    // Loop over process table looking for process to run.
    acquire(&ptable.lock);
    for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
      if(p->state != RUNNABLE)
        continue;

      // Switch to chosen process.  It is the process's job
      // to release ptable.lock and then reacquire it
      // before jumping back to us.
      c->proc = p;
      switchuvm(p);
      p->state = RUNNING;

      swtch(&(c->scheduler), p->context);
      switchkvm();

      // Process is done running for now.
      // It should have changed its p->state before coming back.
      c->proc = 0;
    }
    release(&ptable.lock);

  }
}

// Enter scheduler.  Must hold only ptable.lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->ncli, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&ptable.lock))
    panic("sched ptable.lock");
  if(mycpu()->ncli != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(readeflags()&FL_IF)
    panic("sched interruptible");
  intena = mycpu()->intena;
  swtch(&p->context, mycpu()->scheduler);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  acquire(&ptable.lock);  //DOC: yieldlock
  myproc()->state = RUNNABLE;
  sched();
  release(&ptable.lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch here.  "Return" to user space.
void
forkret(void)
{
  static int first = 1;
  // Still holding ptable.lock from scheduler.
  release(&ptable.lock);

  if (first) {
    // Some initialization functions must be run in the context
    // of a regular process (e.g., they call sleep), and thus cannot
    // be run from main().
    first = 0;
    iinit(ROOTDEV);
    initlog(ROOTDEV);
  }

  // Return to "caller", actually trapret (see allocproc).
}


// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  // if(p->pid == 4) {
  //   cprintf("hello\n");
  // }
  if(p == 0)
    panic("sleep");

  if(lk == 0)
    panic("sleep without lk");

  // Must acquire ptable.lock in order to
  // change p->state and then call sched.
  // Once we hold ptable.lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup runs with ptable.lock locked),
  // so it's okay to release lk.
  if(lk != &ptable.lock){  //DOC: sleeplock0
    acquire(&ptable.lock);  //DOC: sleeplock1
    release(lk);
  }
  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &ptable.lock){  //DOC: sleeplock2
    release(&ptable.lock);
    acquire(lk);
  }
}

//PAGEBREAK!
// Wake up all processes sleeping on chan.
// The ptable lock must be held.
static void
wakeup1(void *chan)
{
  struct proc *p;

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++)
    if(p->state == SLEEPING && p->chan == chan)
      p->state = RUNNABLE;
}

// Wake up all processes sleeping on chan.
void
wakeup(void *chan)
{
  acquire(&ptable.lock);
  wakeup1(chan);
  release(&ptable.lock);
}

// Kill the process with the given pid.
// Process won't exit until it returns
// to user space (see trap in trap.c).
int
kill(int pid)
{
  struct proc *p, *q;

  acquire(&ptable.lock);
  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->pid == pid){
      p->killed = 1;
      // killing the threads of the process if exists
      if(p->isthread == 0) {
        for(q = ptable.proc; q < &ptable.proc[NPROC]; q++){
          if(q->isthread == 1 && q->parent == p){
            q->killed = 1;
            // Wake process from sleep if necessary.
            if(q->state == SLEEPING)
              q->state = RUNNABLE;
          }
        }
      }
      // Wake process from sleep if necessary.
      if(p->state == SLEEPING)
        p->state = RUNNABLE;
      release(&ptable.lock);
      return 0;
    }
  }
  release(&ptable.lock);
  return -1;
}

//PAGEBREAK: 36
// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [EMBRYO]    "embryo",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  int i;
  struct proc *p;
  char *state;
  uint pc[10];

  for(p = ptable.proc; p < &ptable.proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    cprintf("%d %s %s", p->pid, state, p->name);
    if(p->state == SLEEPING){
      getcallerpcs((uint*)p->context->ebp+2, pc);
      for(i=0; i<10 && pc[i] != 0; i++)
        cprintf(" %p", pc[i]);
    }
    cprintf("\n");
  }
}
