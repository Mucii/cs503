#include <xinu.h>

// this is for page falut handler

int32 pg_fault_handler(void){
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

	kprintf("call fault\n");

	count_faults++;

	//Get the faulted address a.
	cr2 = read_cr2();
	fault_vd = (vd_t *)(&cr2);

	kprintf("fault addrees is %u\n",cr2);

	//Let vp be the virtual page number of the page containing the faulted address.
	vpn = VD_TO_VPN(cr2);

	//Let pd point to the current page directory.
	curr_pd = proctab[currpid].prpdptr;

	kprintf("pt is %d, %d\n",(uint32)curr_pd, curr_pd[fault_vd->pd_offset].pd_pres);

	curr_pt = (pt_t*)VPN_TO_VD(curr_pd[fault_vd->pd_offset].pd_base);

	kprintf("pg is %d, %d\n",(uint32)(curr_pd[fault_vd->pd_offset].pd_base), (curr_pt[fault_vd->pt_offset].pt_pres));


	//Check that a is a valid address, if not, print an error message and kill the process.
	if((curr_bs_map=get_bs_map(currpid, vpn)) == NULL){
		kprintf("invalid address\n");
		kill(currpid);
		restore(mask);
		return SYSERR;
	}

	//Let pt point to the p-th page table. If the p-th page table does not exist, obtain a frame for it and initialize it.
	if(curr_pd[fault_vd->pd_offset].pd_pres==0){
		if((curr_pt = pt_allocate())==NULL){
			kill(currpid);
			restore(mask);
			return SYSERR;
		}
		curr_pd[fault_vd->pd_offset].pd_pres = 1;
		curr_pd[fault_vd->pd_offset].pd_base = VD_TO_VPN(curr_pt);
	}else{
		curr_pt = (pt_t*)VPN_TO_VD(curr_pd[fault_vd->pd_offset].pd_base);
	}

	//To bring in the faulted page, do the following:

	//Using the backing store map, find the store s and page offset o which correspond to vp.
	pg_offset = vpn - curr_bs_map->vpn;

	//In the inverted page table, increment the reference count of the frame which holds pt. 
	//This indicates that one more of pt’s entries is marked “present”.
	frameid = VD_TO_FID(curr_pt);
	inverted_page_tab[frameid].refcount++;


	//Obtain a free frame
	new_frameid = frame_allocate();
	frame_tab[frameid].type = FRAME_PG;

	inverted_page_tab[frameid].vpn = vpn;
	inverted_page_tab[frameid].pid = currpid;
	

	//Copy the page o of store s to f
    open_bs(curr_bs_map->bs_id);
    read_bs((char *)(FID_TO_VD(new_frameid)),curr_bs_map->bs_id,pg_offset);
    close_bs(curr_bs_map->bs_id);

    //Update pt to mark the appropriate entry as present
    curr_pt[fault_vd->pt_offset].pt_pres = 1;
    curr_pt[fault_vd->pt_offset].pt_base = FID_TO_VPN(new_frameid);


    restore(mask);
    return OK;

}