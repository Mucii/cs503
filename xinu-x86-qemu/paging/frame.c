#include <xinu.h>

/*this is for frame initial and frame aloocate*/

void frame_initial(void){
	int i;
	intmask mask;
	frame_t *frame_now;
	inverted_page *inverted_page_now;

	mask = disable();
	frame_head = (frame_t *)NULL;

	for(i=0; i<NFRAMES; i++){
		
		// initial frame
		frame_now = &frame_tab[i];
		frame_now->frame_id = i;
		frame_now->state = FRAME_FREE;
		frame_now->type = -1;
		frame_now->next_frame = (frame_t *)NULL;
		frame_now->dirty = 0;


		// intial inverted page table
		inverted_page_now = &inverted_page_tab[i];
		inverted_page_now->refcount = 0;
		inverted_page_now->pid = -1;
		inverted_page_now->vpn = 0;
	}
	restore(mask);
	return ;
}

int32 rm_frame_fifo(int32 frameid){
	intmask mask;
	frame_t *prev_frame;
	frame_t *frame_now;

	mask = disable();

	frame_now = frame_head;
	prev_frame = NULL;
	while(frame_now != NULL){
		if(frame_now->frame_id == frameid){
			// remove frame from the linked list
			if(prev_frame == NULL){
				//adjust head
				frame_head = frame_now->next_frame;
				frame_now->next_frame = (frame_t *)NULL;
			}else{
				prev_frame->next_frame = frame_now->next_frame;
				frame_now->next_frame = (frame_t *)NULL;
			}
			frame_now->state = FRAME_FREE;
			frame_now->type = -1;
			frame_now->next_frame = (frame_t *)NULL;
			restore(mask);
			return OK;
		}
		prev_frame = frame_now;
		frame_now = frame_now->next_frame;
	}
	return SYSERR;
}

int32 get_free_frame_fifo(void){
	intmask mask;
	int32 frameid;
	frame_t *frame_now;

	mask=disable();

	frame_now = frame_head;

	while(frame_now != NULL){
		//only replace pg
		if(frame_now->type == FRAME_PG){
			frameid = frame_now->frame_id;
			rm_frame_fifo(frameid);
			restore(mask);
			return frameid;
		}
		frame_now = frame_now->next_frame;
	}
	restore(mask);
	return SYSERR;
}

int32 get_free_frame_gca(void){
	intmask mask;
	int32 curr_frameid = (last_frameid+1)%NFRAMES;
	frame_t *curr_frame;
	uint32 vpn;
	uint32 vd;
	vd_t *fault_vd;
	pd_t *curr_pd;
	pt_t *curr_pt;
	int32 i;

	mask=disable();
	// at most go over 3 times
	for(i=0; i<4*NFRAMES; i++){
		curr_frame = &frame_tab[curr_frameid];
		//here would be not free frame
		// if the frame is for page
		if(curr_frame->type == FRAME_PG){
			vpn = inverted_page_tab[curr_frameid].vpn;
			vd = VPN_TO_VD(vpn);
			fault_vd = (vd_t *)(&vd);
			curr_pd = proctab[currpid].prpdptr;
			curr_pt = (pt_t*)VPN_TO_VD(curr_pd[fault_vd->pd_offset].pd_base);
			// (0,0) case
			if((curr_pt[fault_vd->pt_offset].pt_acc == 0) && (curr_pt[fault_vd->pt_offset].pt_dirty == 0)){
				curr_frame->type = -1;
				last_frameid = curr_frameid;
				rm_frame_fifo(curr_frameid);
				restore(mask);
				return curr_frameid;
			// （1，0） case
			}else if((curr_pt[fault_vd->pt_offset].pt_acc == 1) && (curr_pt[fault_vd->pt_offset].pt_dirty == 0)){
				curr_pt[fault_vd->pt_offset].pt_acc = 0;
			// （1，1） case
			}else if((curr_pt[fault_vd->pt_offset].pt_acc == 1) && (curr_pt[fault_vd->pt_offset].pt_dirty == 1)){
				curr_pt[fault_vd->pt_offset].pt_dirty =0;
				curr_frame->dirty = 1;
			}
		}

		curr_frameid = (curr_frameid+1)%NFRAMES;

	}
	restore(mask);
	return SYSERR;
}



int32 free_frame(int32 frame_id){
	// free the frame
	intmask mask;
	uint32 curr_vpn; 
	pid32 curr_pid;
	vd_t *virtual_address;
	pd_t *curr_pd;
	pt_t *curr_pt;
	int32 pt_frame_id;
	uint32 offset;
	uint32 vaddress;

	mask = disable();

	//get vpn and pid
	curr_vpn = inverted_page_tab[frame_id].vpn;
	curr_pid = inverted_page_tab[frame_id].pid;
	//get vir address
	vaddress =  VPN_TO_VD(curr_vpn);
	virtual_address = (vd_t *)(&vaddress);
	//get curr pd
	curr_pd = proctab[curr_pid].prpdptr;

	//get curr pt 
	curr_pt = (pt_t*)VPN_TO_VD(curr_pd[virtual_address->pd_offset].pd_base);

	//Mark the appropriate entry of pt as not present.
	curr_pt[virtual_address->pt_offset].pt_pres = 0;

	//If the page being removed belongs to the current process, invalidate the TLB entry for the page 

	if(curr_pid == currpid){
		// check invlpg;
		tmp = curr_vpn;
		//kprintf("same process\n");
		asm("pushl %eax");
		asm("invlpg tmp");
		asm("popl %eax");
	}

	//In the inverted page table, decrement the reference count of the frame occupied by pt.
	pt_frame_id = curr_pd[virtual_address->pd_offset].pd_base - FRAME0;

	inverted_page_tab[pt_frame_id].refcount--;

	//If the reference count has reached zero, you should mark the appropriate entry in pd as “not present.
	if(inverted_page_tab[pt_frame_id].refcount == 0){
		curr_pd[virtual_address->pd_offset].pd_pres = 0;
		// clean the frame of pt table
		hook_ptable_delete(pt_frame_id);

		// remove frame from fifo list
		rm_frame_fifo(pt_frame_id);
	}


	//If the dirty bit for page vp was set in its page table,
	//second is only for gca

	if(curr_pt[virtual_address->pt_offset].pt_dirty == 1 || frame_tab[frame_id].dirty == 1){
		bs_map *curr_bs_map;

		//Using the backing store map
		if((curr_bs_map = get_bs_map(curr_pid,curr_vpn))==NULL){
			kprintf("some thing wrong when find bs\n");
			kill(curr_pid);
            restore(mask);
            return SYSERR;
		}

		// Write the page back to the backing store. 
		offset = curr_vpn - curr_bs_map->vpn;
		if(open_bs(curr_bs_map->bs_id)==SYSERR){
			kill(curr_pid);
            restore(mask);
            return SYSERR;
		}

		//for network error
		if(write_bs((char*)FID_TO_VD(frame_id), curr_bs_map->bs_id, offset)==SYSERR){
			kill(curr_pid);
            restore(mask);
            return SYSERR;
		}


		if(close_bs(curr_bs_map->bs_id)==SYSERR){
			kill(curr_pid);
            restore(mask);
            return SYSERR;
        }
    }
	hook_pswap_out(curr_pid,curr_vpn,frame_id);
	restore(mask);
	return OK;
}


int32 get_free_frame(void){
	// find a free frame
	intmask mask;
	int32 frameid;
	frame_t *frame_now;
	int32 i;

	mask = disable();

	// go over frame-tab to find free frame
	for(i=0; i<NFRAMES; i++){
		frame_now = &frame_tab[i];
		if(frame_now->state==FRAME_FREE){
			frameid = i;
			restore(mask);
			return frameid;
		}
	}

	// check replace policy and replace 
	if(currpolicy == FIFO){
		frameid = get_free_frame_fifo();
	}else if(currpolicy == GCA){
		frameid = get_free_frame_gca();
	}else{
		restore(mask);
		return SYSERR;
	}
	//kprintf("frameid return is %d\n",frameid);
	//rm_frame_fifo(frameid);

	// ensure frameid is a valid number
	if(frameid == SYSERR){
		restore(mask);
		return SYSERR;
	}

	free_frame(frameid);
	restore(mask);
	return frameid;

}

int32 frame_allocate(void){
	intmask mask;
	frame_t *curr_frame;
	frame_t *temp_frame;
	inverted_page *curr_inverted_page;
	int32 frameid;

	// frametab is shared data which can only be accessed by one process
	mask = disable();
	//kprintf("start get frame\n");
	if((frameid=get_free_frame()) == SYSERR){
		//kprintf("find it 1\n");
		restore(mask);
		return SYSERR;
	}
	//kprintf("end get frame\n");

	// set frame
	curr_frame = &frame_tab[frameid];
	curr_frame->state = FRAME_USED;
	curr_frame->type = -1;
	curr_frame->dirty = 0;
	curr_frame->next_frame = (frame_t *)NULL;
	// set inverted page
	curr_inverted_page = &inverted_page_tab[frameid];
	curr_inverted_page->refcount = 0;
	curr_inverted_page->pid=currpid; 

	//kprintf("start to find frame %d pid %d\n",curr_frame->frame_id,currpid);
	// put frame into frame linked list
	if(frame_head==NULL){
		frame_head = curr_frame;
	}else{
		temp_frame = frame_head;

		while(temp_frame->next_frame != NULL){
			//kprintf("add frame %d\n",temp_frame->frame_id);
			temp_frame = temp_frame->next_frame;
		}

		temp_frame->next_frame = curr_frame;
		//kprintf("add frame %d\n",temp_frame->frame_id);
		//kprintf("frame %d\n",curr_frame->frame_id);
	}

	//kprintf("find it 6\n");
	restore(mask);

	return frameid;
}


