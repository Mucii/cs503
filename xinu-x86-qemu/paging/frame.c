#include <xinu.h>

/*this is for frame initial and frame aloocate*/

void frame_initial(void){
	int i;
	frame_t *frame_now;
	inverted_page *inverted_page_now;
	frame_head = (frame_t *)NULL;

	for(i=0; i<NFRAMES; i++){
		
		// initial frame
		frame_now = &frame_tab[i];
		frame_now->frame_id = i;
		frame_now->state = FRAME_FREE;
		frame_now->type = -1;
		frame_now->next_frame = (frame_t *)NULL;


		// intial inverted page table
		inverted_page_now = &inverted_page_tab[i];
		inverted_page_now->refcount = 0;
		inverted_page_now->pid = -1;
		inverted_page_now->vpn = 0;
	}

	return ;
}

int32 get_free_frame_fifo(void){
	intmask mask;
	int32 frameid;
	frame_t *prev_frame;
	frame_t *frame_now;

	mask=disable();

	frame_now = frame_head;
	prev_frame = NULL;

	while(frame_now != NULL){
		//only replace pg
		if(frame_now->type == FRAME_PG){
			frameid = frame_now->frame_id;

			// remove frame from the linked list
			if(prev_frame == NULL){
				//adjust head
				frame_head = frame_now->next_frame;
				frame_now->next_frame = (frame_t *)NULL;
			}else{
				prev_frame->next_frame = frame_now->next_frame;
				frame_now->next_frame = (frame_t *)NULL;
			}
			restore(mask);
			return frameid;
		}
		prev_frame = frame_now;
		frame_now = frame_now->next_frame;
	}
	restore(mask);
	return SYSERR;
}


int32 get_free_frame_gca(void){
	// to be finished
	return SYSERR;
}

int32 rm_frame_fifo(int32 frameid){
	frame_t *prev_frame;
	frame_t *frame_now;

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
			break;
		}
		prev_frame = frame_now;
		frame_now = frame_now->next_frame;
		}

	frame_now = &frame_tab[frameid];

	frame_now->state = FRAME_FREE;
	frame_now->type = -1;
	frame_now->next_frame = (frame_t *)NULL;

	return OK;

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
	mask = disable();

	//get vpn and pid
	curr_vpn = inverted_page_tab[frame_id].vpn;
	curr_pid = inverted_page_tab[frame_id].pid;


	//get vir address
	virtual_address = (vd_t*)VPN_TO_VD(curr_vpn);

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
		asm("invlpg tmp");
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

	if(curr_pt[virtual_address->pt_offset].pt_dirty == 1){
		bs_map *curr_bs_map;

		//Using the backing store map
		if((curr_bs_map = get_bs_map(curr_pid,curr_vpn))==NULL){
			kprintf("some thing wrong when find bs\n");
			kill(curr_pid);
            restore(mask);
            return SYSERR;
		}

		// Write the page back to the backing store. 
		uint32 offset = curr_vpn - curr_bs_map->vpn;

		open_bs(curr_bs_map->bs_id);
		write_bs((char*)((FRAME0 + frame_id) * NBPG), curr_bs_map->bs_id, offset);
		close_bs(curr_bs_map->bs_id);

	}

	restore(mask);
	return OK;
}


int32 get_free_frame(void){
	// find a free frame
	intmask mask;
	int32 frameid;
	frame_t *frame_now;
	int i;

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

	if((frameid=get_free_frame()) == SYSERR){
		restore(mask);
		return SYSERR;
	}

	// set frame

	curr_frame = &frame_tab[frameid];
	curr_frame->state = FRAME_USED;
	curr_frame->type = -1;
	curr_frame->next_frame = (frame_t *)NULL;

	// set inverted page
	curr_inverted_page = &inverted_page_tab[frameid];
	curr_inverted_page->refcount = 0;
	curr_inverted_page->pid=currpid; 


	// put frame into frame linked list

	if(frame_head==NULL){
		frame_head = curr_frame;
	}else{
		temp_frame = frame_head;

		while(temp_frame->next_frame != NULL){
			temp_frame = temp_frame->next_frame;
		}

		temp_frame->next_frame = curr_frame;
	}
	kprintf("frame id is %d\n",frameid);
	restore(mask);

	return frameid;
}


