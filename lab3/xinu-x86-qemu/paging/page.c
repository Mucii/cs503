#include <xinu.h>

/*this is for page initial and page aloocate*/
pt_t *global_pt[4];
pt_t *device_pt;


pt_t* pt_allocate(void){
	// allocate a new page table
	int32 frameid;
	int32 i;
	frame_t *curr_frame;
	pt_t *curr_pt;

	if((frameid = frame_allocate()) == SYSERR){
		return (pt_t*) NULL;
	}

	// find frame and change type
	curr_frame = &frame_tab[frameid];

	curr_frame->type = FRAME_PT;

	//kprintf("type is %d\n",frame_tab[frameid].type);

	// clean pt
	curr_pt = (pt_t*)FID_TO_VD(frameid);


	for(i = 0; i<PAGETABSIZE; i++){
		curr_pt[i].pt_pres  = 0;		/* page is present?		*/
	    curr_pt[i].pt_write = 1;		/* page is writable?		*/
		curr_pt[i].pt_user	= 0;		/* is use level protection?	*/
		curr_pt[i].pt_pwt	= 0;		/* write through for this page? */
		curr_pt[i].pt_pcd	= 0;		/* cache disable for this page? */
		curr_pt[i].pt_acc	= 0;		/* page was accessed?		*/
		curr_pt[i].pt_dirty = 0;		/* page was written?		*/
		curr_pt[i].pt_mbz	= 0;		/* must be zero			*/
		curr_pt[i].pt_global= 0;		/* should be zero in 586	*/
		curr_pt[i].pt_avail = 0;		/* for programmer's use		*/
		curr_pt[i].pt_base = 0;         // this will be aloocated when page is created
	}
	//kprintf("create pt frame is %d\n",frameid);
	hook_ptable_create(frameid);
	return curr_pt;
}


int32 init_global_pt(void){
	//initial global page table
	pt_t * curr_pt;

	int i;
	int j;

	for(i=0; i<4; i++){
		curr_pt = pt_allocate();

		if(curr_pt == NULL){
			return SYSERR;
		}
		for(j=0; j<PAGETABSIZE; j++){
			curr_pt[j].pt_pres  = 1;		/* page is present?		*/
	    	curr_pt[j].pt_write = 1;		/* page is writable?		*/
			curr_pt[j].pt_user	= 0;		/* is use level protection?	*/
			curr_pt[j].pt_pwt	= 0;		/* write through for this page? */
			curr_pt[j].pt_pcd	= 0;		/* cache disable for this page? */
			curr_pt[j].pt_acc	= 0;		/* page was accessed?		*/
			curr_pt[j].pt_dirty = 0;		/* page was written?		*/
			curr_pt[j].pt_mbz	= 0;		/* must be zero			*/
			curr_pt[j].pt_global= 0;		/* should be zero in 586	*/
			curr_pt[j].pt_avail = 0;		/* for programmer's use		*/

			// You can make others zero
			curr_pt[j].pt_base = i * PAGETABSIZE + j;         // this will be aloocated when page is created
		}
		global_pt[i] = curr_pt;
	}

	curr_pt = pt_allocate();

	if(curr_pt == NULL){
		return SYSERR;
	}
	for(i=0; i<PAGETABSIZE; i++){
		curr_pt[i].pt_pres  = 1;		/* page is present?		*/
	    curr_pt[i].pt_write = 1;		/* page is writable?		*/
		curr_pt[i].pt_user	= 0;		/* is use level protection?	*/
		curr_pt[i].pt_pwt	= 0;		/* write through for this page? */
		curr_pt[i].pt_pcd	= 0;		/* cache disable for this page? */
		curr_pt[i].pt_acc	= 0;		/* page was accessed?		*/
		curr_pt[i].pt_dirty = 0;		/* page was written?		*/
		curr_pt[i].pt_mbz	= 0;		/* must be zero			*/
		curr_pt[i].pt_global= 0;		/* should be zero in 586	*/
		curr_pt[i].pt_avail = 0;		/* for programmer's use		*/
		curr_pt[i].pt_base = DEV_VPN + i;         // from 589824 
	}
	device_pt = curr_pt;

	return OK;
}


pd_t* pd_allocate(void){
	// allocate a new page table
	int32 frameid;
	int32 i;
	frame_t *curr_frame;
	pd_t *curr_pd;


	if((frameid = frame_allocate()) == SYSERR){
		return (pd_t*) NULL;
	}

	// find frame and change type
	curr_frame = &frame_tab[frameid];

	curr_frame->type = FRAME_PD;

	// clean pd
	curr_pd = (pd_t*)FID_TO_VD(frameid);


	for(i = 0; i<PAGEDIRSIZE; i++){
		curr_pd[i].pd_pres	= 0;		/* page table present?		*/
		curr_pd[i].pd_write = 1;		/* page is writable?		*/
		curr_pd[i].pd_user	= 0;		/* is use level protection?	*/
		curr_pd[i].pd_pwt	= 0;		/* write through cachine for pt? */
		curr_pd[i].pd_pcd	= 0;		/* cache disable for this pt?	*/
		curr_pd[i].pd_acc	= 0;		/* page table was accessed?	*/
		curr_pd[i].pd_mbz	= 0;		/* must be zero			*/
		curr_pd[i].pd_fmb	= 0;		/* four MB pages?		*/
		curr_pd[i].pd_global= 0;
		curr_pd[i].pd_avail = 0;
		curr_pd[i].pd_base  = 0;
	}
	
	// set global page table
	for(i =0; i<4; i++){
		curr_pd[i].pd_pres = 1;  //set present
		curr_pd[i].pd_write = 1;
		curr_pd[i].pd_base = VD_TO_VPN(global_pt[i]);

		//kprintf("ptable is %d\n", (uint32)global_pt[i]);
	}

	// set dev page table

	curr_pd[DEV_PTN].pd_pres = 1;
	curr_pd[DEV_PTN].pd_write = 1;
	curr_pd[DEV_PTN].pd_base = VD_TO_VPN(device_pt);

	return curr_pd;
}