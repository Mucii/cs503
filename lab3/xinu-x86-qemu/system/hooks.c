/* hooks.c - hook_ptable_create, hook_ptable_delete, hook_pfault, hook_pswap_out */

#include <xinu.h>

#define HOOK_LOG_ON

#define POLICY_TESTING 0

int32 last = 0;

/*
 * Note that this file will be replaced in grading. So you may change them to
 * output what you are interested, but do not let your implementation depend on
 * this file.
 */

/*---------------------------------------------------------------------------
 *  hook_ptable_create  -  Called when your implementation is creating a page
 *  table.
 *---------------------------------------------------------------------------
 */
void hook_ptable_create(uint32 pagenum) {
#ifdef HOOK_LOG_ON
	kprintf("\n=== Created page table %d ===\n", pagenum);
#endif
}

/*---------------------------------------------------------------------------
 *  hook_ptable_delete  -  Called when your implementation is deleting a page
 *  table.
 *---------------------------------------------------------------------------
 */
void hook_ptable_delete(uint32 pagenum) {
#ifdef HOOK_LOG_ON
	kprintf("\n=== Deleted page table %d ===\n", pagenum);
#endif
}

/*---------------------------------------------------------------------------
 *  hook_pfault  -  Called whenever the page fault handler is called.
 *---------------------------------------------------------------------------
 */
void hook_pfault(int16 procid, void *addr, uint32 pagenum, uint32 framenum) {
#ifdef HOOK_LOG_ON
	kprintf("\n=== Page fault for address 0x%0X ===\n", addr);

  if (isbadpid(currpid)) {
    kprintf("[ERR] bad pid\n");
    panic("[FAIL]\n");
  }

#if POLICY_TESTING == 1
  // Add to the end of the list (ADIL)
  int16 cf = frame_md.curframe;
  frame_md.alloc_page_proc[cf] = procid;
  frame_md.alloc_page[cf] = pagenum;
  frame_md.curframe++;
  if (frame_md.curframe == NFRAMES) frame_md.curframe = 0;
  kprintf("Procid: %d, Framenum: %d, Pagenum: %d\n", procid, framenum, pagenum);
#endif

#endif
}

/*---------------------------------------------------------------------------
 *  hook_pswap_out  -  Called when your implementation is replacing (swapping
 *  out) a frame.
 *---------------------------------------------------------------------------
 */
void hook_pswap_out(int16 procid, uint32 pagenum, uint32 framenum) {
#ifdef HOOK_LOG_ON
  // Based on the current policy, check if the page-number and framenumber being
  // replaced is accurate (ADIL)

  if (isbadpid(currpid)) {
    kprintf("[ERR] bad pid\n");
    panic("FAIL\n");
  }

	kprintf("\n=== Page Replacement  ===\n");
  kprintf("=== Frame: %d, Process-id: %d, Page-Number: %d ===\n", framenum, procid, pagenum);

#if POLICY_TESTING == 1
  if (currpolicy == FIFO) {
    kprintf("Testing FIFO .. ");

    int16 recf = frame_md.reclaimframe;
    uint32 correct_pid = frame_md.alloc_page_proc[recf];
    uint32 correct_page = frame_md.alloc_page[recf];
    if (correct_page != pagenum || procid != correct_pid) {
      kprintf("[ERR] policy is not being followed\n");
      panic("FAIL\n");
    }
    frame_md.reclaimframe++;
    if (frame_md.reclaimframe == NFRAMES) frame_md.reclaimframe = 0;

    kprintf("Passed\n");
  } else {
    kprintf("Testing GCA .. ");
    // this is my own test for GCA 
    /*int32 i = (last+1)%NFRAMES;

    uint32 vd;
    uint32 vpn;
    vd_t *fault_vd;
    pd_t *curr_pd;
    pt_t *curr_pt;

    while(i != framenum){
      vpn = inverted_page_tab[i].vpn;
      vd = VPN_TO_VD(vpn);
      fault_vd = (vd_t *)(&vd);
      curr_pd = proctab[currpid].prpdptr;
      curr_pt = (pt_t*)VPN_TO_VD(curr_pd[fault_vd->pd_offset].pd_base);
      
      if(curr_pt[fault_vd->pt_offset].pt_acc == 0 && curr_pt[fault_vd->pt_offset].pt_dirty != 0){
        panic("FAIL\n");
      }
      i=(i+1)%NFRAMES;
    }
    
    vd = VPN_TO_VD(pagenum);
    fault_vd = (vd_t *)(&vd);
    curr_pd = proctab[currpid].prpdptr;
    curr_pt = (pt_t*)VPN_TO_VD(curr_pd[fault_vd->pd_offset].pd_base);

    if(curr_pt[fault_vd->pt_offset].pt_acc != 0 || curr_pt[fault_vd->pt_offset].pt_dirty != 0){
      panic("FAIL\n");
    }

    last = last_frameid;*/

    kprintf("Passed\n");
  }
#endif


#endif
}
