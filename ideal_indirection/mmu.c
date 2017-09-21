/**
 * Ideal Indirection Lab
 * CS 241 - Spring 2017
 */
#include "kernel.h"
#include "mmu.h"
#include <assert.h>
#include <stdio.h>

MMU *MMU_create() {
  MMU *mmu = calloc(1, sizeof(MMU));
  mmu->tlb = TLB_create();
  mmu->curr_pid = 0;
  return mmu;
}

void *MMU_get_physical_address(MMU *mmu, void *virtual_address, size_t pid) {
  assert(pid < MAX_PROCESS_ID);
  // TODO: implement this function!
  size_t offset = ((size_t)virtual_address) & 0x000007fff;
  size_t TLB_address = ((size_t)virtual_address << 13) >> 28;
  size_t VPN1_address = ((size_t)virtual_address >> 39) & 0x000000fff;
  size_t VPN2_address = ((size_t)virtual_address >> 27) & 0x000000fff;
  size_t VPN3_address = ((size_t)virtual_address >> 15) & 0x000000fff;

  if(mmu->curr_pid != pid){
    TLB_flush(&(mmu->tlb));
    mmu->curr_pid = pid;
  }
  void* TLB_temp = TLB_get_physical_address(&(mmu->tlb), (void*)TLB_address);
  if(TLB_temp){
    return TLB_temp + offset;
  }
  MMU_tlb_miss(mmu, virtual_address, pid);
  
  

  void* entry1 = NULL;
  void* entry2 = NULL;
  void* entry3 = NULL;

  entry1 = PageTable_get_entry(mmu->base_pts[pid], VPN1_address);
  if(entry1){
    entry2 = PageTable_get_entry((PageTable*)entry1, VPN2_address);
  }else{
    MMU_raise_page_fault(mmu, virtual_address, pid);
    entry1 = PageTable_create();
    PageTable_set_entry(mmu->base_pts[pid], VPN1_address, (PageTable*)entry1);
    //entry2 = PageTable_get_entry((PageTable*)entry1, VPN2_address);
  }
  if(entry2){
    entry3 = PageTable_get_entry((PageTable*)entry2, VPN3_address);
  }else{
    MMU_raise_page_fault(mmu, virtual_address, pid);
    entry2 = PageTable_create();
    PageTable_set_entry((PageTable*)entry1, VPN2_address, (PageTable*)entry2);
    //entry3 = PageTable_get_entry((PageTable*)entry2, VPN3_address);
  }
  if(entry3){
    return entry3 + offset;
  }else{
    MMU_raise_page_fault(mmu, virtual_address, pid);
    entry3 = ask_kernel_for_frame();
    PageTable_set_entry((PageTable*)entry2, VPN3_address, (PageTable*)entry3);
    TLB_add_physical_address(&(mmu->tlb), (void*)TLB_address, entry3);
    return entry3 + offset;
  }
  return NULL;
}

void MMU_tlb_miss(MMU *mmu, void *address, size_t pid) {
  assert(pid < MAX_PROCESS_ID);
  mmu->num_tlb_misses++;
  printf("Process [%lu] tried to access [%p] and it couldn't find it in the "
         "TLB so the MMU has to check the PAGE TABLES\n",
         pid, address);
}

void MMU_raise_page_fault(MMU *mmu, void *address, size_t pid) {
  assert(pid < MAX_PROCESS_ID);
  mmu->num_page_faults++;
  printf(
      "Process [%lu] tried to access [%p] and the MMU got an invalid entry\n",
      pid, address);
}

void MMU_add_process(MMU *mmu, size_t pid) {
  assert(pid < MAX_PROCESS_ID);
  mmu->base_pts[pid] = PageTable_create();
}

void MMU_free_process_tables(MMU *mmu, size_t pid) {
  assert(pid < MAX_PROCESS_ID);
  PageTable *base_pt = mmu->base_pts[pid];
  Pagetable_delete_tree(base_pt);
}

void MMU_delete(MMU *mmu) {
  for (size_t i = 0; i < MAX_PROCESS_ID; i++) {
    MMU_free_process_tables(mmu, i);
  }
  TLB_delete(mmu->tlb);
  free(mmu);
}
