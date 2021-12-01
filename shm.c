#include "param.h"
#include "types.h"
#include "defs.h"
#include "x86.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "spinlock.h"

struct {
  struct spinlock lock;
  struct shm_page {
    uint id;
    char *frame;
    int refcnt;
  } shm_pages[64];
} shm_table;

void shminit() {
  int i;
  initlock(&(shm_table.lock), "SHM lock");
  acquire(&(shm_table.lock));
  for (i = 0; i< 64; i++) {
    shm_table.shm_pages[i].id =0;
    shm_table.shm_pages[i].frame =0;
    shm_table.shm_pages[i].refcnt =0;
  }
  release(&(shm_table.lock));
}

int shm_open(int id, char **pointer) {
//shm_open looks through the shm_table to see if this segment id already exists. 
//If it doesn’t then it needs to allocate a page and map it, and store this information in the shm_table. 
//If the segment already exists, increase the refence count, and add the mapping between the virtual address and the physical address. 
//In either case, return the virtual address through the second parameter of the system call.
//you write this
int i;
//acquiring shared memory locks.
acquire(&(shm_table.lock));
//Case 1
//If the segment id already exists
for(i = 0; i < 64; i++){
  if(shm_table.shm_pages[i].id == id){
    //map the physical address of the page in the table to an available page in virtual address space
    mappages(myproc()->pgdir, (char*) PGROUNDUP(myproc()->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
    //Increase reference count
    shm_table.shm_pages[i].refcnt++;
    //return pointer to the virtual address
    *pointer = (char*) PGROUNDUP(myproc()->sz);
    //update sz since virtual address space expanded
    myproc()->sz+=PGSIZE;
    //release lock
    release(&(shm_table.lock));
    return 0;
  }
}
//Case 2
//If the segment id does not exist
for(i = 0; i < 64; i++){
  if(shm_table.shm_pages[i].id == 0){
    //initialize the id to the id passed to us
    shm_table.shm_pages[i].id = id;
    //kalloc a page and store its address in frame
    shm_table.shm_pages[i].frame = kalloc();
    //set reference count to 1
    shm_table.shm_pages[i].refcnt = 1;
    //The following is similar to Case 1
    //map the physical address of the page in the table to an available page in virtual address space
    memset(shm_table.shm_pages[i].frame, 0, PGSIZE);
    mappages(myproc()->pgdir, (char*) PGROUNDUP(myproc()->sz), PGSIZE, V2P(shm_table.shm_pages[i].frame), PTE_W|PTE_U);
    //return pointer to the virtual address
    *pointer = (char*) PGROUNDUP(myproc()->sz);
    //update sz since virtual address space expanded
    myproc()->sz+=PGSIZE;
    //release lock
    release(&(shm_table.lock));
    return 0;
  }
}
return 0; //added to remove compiler warning -- you should decide what to return
}


int shm_close(int id) {
//you write this too!
//Look for the shared memory segment, and decreases its reference count
//If it reaches 0, clears the shm_table
int i;
//acquiring shared memory locks.
acquire(&(shm_table.lock));
//Loop through all 64 pages to find the segment
for(i = 0; i < 64; i++){
  if(shm_table.shm_pages[i].id == id){
    //Decrease reference count
    shm_table.shm_pages[i].refcnt--;
    //Cleat it if refcnt reaches 0
    if(shm_table.shm_pages[i].refcnt == 0){
      shm_table.shm_pages[i].id = 0;
      shm_table.shm_pages[i].frame = 0;
      shm_table.shm_pages[i].refcnt = 0;
    }
  }
}
//Release lock
release(&(shm_table.lock));
return 0; //added to remove compiler warning -- you should decide what to return
}
