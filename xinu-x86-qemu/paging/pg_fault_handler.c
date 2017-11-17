#include <xinu.h>

// this is for page falut handler

void pg_fault_handler(void){
	intmask mask;
	unsigned long cr2;
	vd_t *fault_vd;
	uint32 vpn;
	pd_t *curr_pd;
	bs_map *curr_bs_map;
	pt_t *curr_pt;
	uint32 pg_offset;
	int32 frameid;
	int32 new_frameid;



	mask = disable();

	//kprintf("call fault\n");

	count_faults++;

	//Get the faulted address a.
	cr2 = read_cr2();
	//kprintf("read address is 0x%08x\n",cr2);
	fault_vd = (vd_t *)(&cr2);

	//pt_t *gpt;

	//gpt = global_pt[0];

	//kprintf("check pres is %d\n",gpt[0].pt_pres);

	//kprintf("fault addrees is %u\n",cr2);

	//Let vp be the virtual page number of the page containing the faulted address.
	vpn = VD_TO_VPN(cr2);

	//kprintf("read vpn is %d\n", vpn);

	//Let pd point to the current page directory.
	curr_pd = proctab[currpid].prpdptr;

	//kprintf("pt is %d, %d\n",fault_vd->pd_offset, curr_pd[fault_vd->pd_offset].pd_pres);

	//curr_pt = (pt_t*)VPN_TO_VD(curr_pd[fault_vd->pd_offset].pd_base);

	//kprintf("pg is %d, %d\n",(uint32)fault_vd->pt_offset, (curr_pt[fault_vd->pt_offset].pt_pres));

	//kprintf("1\n");
	//Check that a is a valid address, if not, print an error message and kill the process.
	if((curr_bs_map=get_bs_map(currpid, vpn)) == NULL){
		kprintf("invalid address\n");
		kill(currpid);
		restore(mask);
		return;
	}
	//kprintf("2\n");
	//Let pt point to the p-th page table. If the p-th page table does not exist, obtain a frame for it and initialize it.
	if(curr_pd[fault_vd->pd_offset].pd_pres==0){
		if((curr_pt = pt_allocate())==NULL){
			kill(currpid);
			restore(mask);
			return;
		}
		curr_pd[fault_vd->pd_offset].pd_pres = 1;
		curr_pd[fault_vd->pd_offset].pd_base = VD_TO_VPN(curr_pt);
		//kprintf("3\n");
	}else{
		curr_pt = (pt_t*)VPN_TO_VD(curr_pd[fault_vd->pd_offset].pd_base);
		//kprintf("4\n");
	}

	//To bring in the faulted page, do the following:
	//kprintf("5\n");
	//Using the backing store map, find the store s and page offset o which correspond to vp.
	pg_offset = vpn - curr_bs_map->vpn;

	//In the inverted page table, increment the reference count of the frame which holds pt. 
	//This indicates that one more of pt’s entries is marked “present”.
	frameid = VD_TO_FID(curr_pt);
	inverted_page_tab[frameid].refcount++;

	//kprintf("6\n");
	//Obtain a free frame
	new_frameid = frame_allocate();
	frame_tab[new_frameid].type = FRAME_PG;

	inverted_page_tab[new_frameid].vpn = vpn;
	inverted_page_tab[new_frameid].pid = currpid;
	
	//kprintf("7\n");
	//Copy the page o of store s to f
    open_bs(curr_bs_map->bs_id);
    read_bs((char *)FID_TO_VD(new_frameid),curr_bs_map->bs_id,pg_offset);
    close_bs(curr_bs_map->bs_id);
    //kprintf("8\n");
    //Update pt to mark the appropriate entry as present
    curr_pt[fault_vd->pt_offset].pt_pres = 1;
    curr_pt[fault_vd->pt_offset].pt_base = FID_TO_VPN(new_frameid);
    //kprintf("pd is %u\n",fault_vd->pd_offset);
    //kprintf("pt is %u\n",fault_vd->pt_offset);
    //kprintf("offset is %u\n",FID_TO_VPN(new_frameid));  //kprintf("9\n");
    hook_pfault(currpid, (void*)cr2, vpn, new_frameid);

    // magic part i dont know, but pls dont remove
    uint32 *read1 = (uint32 *)cr2;
    pg_offset = *read1;
    //kprintf("read back p is %u\n",*read1);
    restore(mask);
    return;

}