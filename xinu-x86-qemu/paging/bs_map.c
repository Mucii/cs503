#include <xinu.h>

// this is for bs map 


//performs the following lookup
bs_map* get_bs_map(pid32 pid, uint32 vpn){
	intmask mask;
	bs_map *curr_bs_map;
	struct bs_entry *curr_bs;
	int32 i;

	mask = disable();

	for(i=0; i<MAX_BS_ENTRIES; i++){
		curr_bs = &bstab[i];
		curr_bs_map = &bs_map_tab[i];
		// check whether all conditions are met
		if((curr_bs->isallocated == FALSE) || (curr_bs_map->allocated == FALSE) || (curr_bs_map->pid != pid) || (vpn < curr_bs_map->vpn) || (vpn >= (curr_bs_map->vpn+curr_bs_map->npg))){
			continue;
		}else{
			//kprintf("map is %d %d\n",i,currpid);
			restore(mask);
			return curr_bs_map;
		}
	}
	restore(mask);
	return (bs_map*) NULL;
}

// initial bs map tab

void initial_bs_map_tab(void){
	intmask mask;
	int32 i;

	mask = disable();
	for(i=0; i<MAX_BS_ENTRIES; i++){
		bs_map_tab[i].pid = -1;
		bs_map_tab[i].vpn = 0;
		bs_map_tab[i].npg = 0;
		bs_map_tab[i].bs_id = -1;
		bs_map_tab[i].allocated = FALSE;
	}
	restore(mask);
	return;
}

// add a new map
int32 add_bs_map(pid32 pid, uint32 vpn, uint32 npg, bsd_t bs_id){
	intmask mask;
	bs_map *curr_bs_map;

	mask = disable();
	//kprintf("start allocate bs %d\n", bs_id);


	if(bstab[bs_id].isallocated==FALSE || bs_map_tab[bs_id].allocated == TRUE){
		restore(mask);
		return SYSERR;
	}


	//kprintf("begin allocate bs map\n");

	curr_bs_map = &bs_map_tab[bs_id];
	curr_bs_map->pid = pid;
	curr_bs_map->vpn = vpn;
	curr_bs_map->npg = npg;
	curr_bs_map->bs_id = bs_id;
	curr_bs_map->allocated = TRUE;
	restore(mask);
	return OK;

}

// remove a map of a process 

int32 rm_bs_map(pid32 pid){
	intmask mask;
	bs_map *curr_bs_map;
	int32 i;

	mask =disable();

	for(i=0; i<MAX_BS_ENTRIES; i++){
		curr_bs_map = &bs_map_tab[i];
		if(curr_bs_map->allocated== TRUE && curr_bs_map->pid==pid){
			//deallocate bs 
			if(deallocate_bs(curr_bs_map->bs_id)==SYSERR){
				restore(mask);
				return SYSERR;
			};
			//reinitial bs map
			curr_bs_map->pid = -1;
			curr_bs_map->vpn = 0;
			curr_bs_map->npg = 0;
			curr_bs_map->bs_id = -1;
			curr_bs_map->allocated = FALSE;
		}
	}
	restore(mask);
	return OK;
}