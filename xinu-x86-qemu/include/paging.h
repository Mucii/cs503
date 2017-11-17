/* paging.h */

#ifndef __PAGING_H_
#define __PAGING_H_

/* Structure for a page directory entry */

typedef struct {
	unsigned int pd_pres	: 1;		/* page table present?		*/
	unsigned int pd_write : 1;		/* page is writable?		*/
	unsigned int pd_user	: 1;		/* is use level protection?	*/
	unsigned int pd_pwt	: 1;		/* write through cachine for pt? */
	unsigned int pd_pcd	: 1;		/* cache disable for this pt?	*/
	unsigned int pd_acc	: 1;		/* page table was accessed?	*/
	unsigned int pd_mbz	: 1;		/* must be zero			*/
	unsigned int pd_fmb	: 1;		/* four MB pages?		*/
	unsigned int pd_global: 1;		/* global (ignored)		*/
	unsigned int pd_avail : 3;		/* for programmer's use		*/
	unsigned int pd_base	: 20;		/* location of page table?	*/
} pd_t;

/* Structure for a page table entry */

typedef struct {
	unsigned int pt_pres	: 1;		/* page is present?		*/
	unsigned int pt_write : 1;		/* page is writable?		*/
	unsigned int pt_user	: 1;		/* is use level protection?	*/
	unsigned int pt_pwt	: 1;		/* write through for this page? */
	unsigned int pt_pcd	: 1;		/* cache disable for this page? */
	unsigned int pt_acc	: 1;		/* page was accessed?		*/
	unsigned int pt_dirty : 1;		/* page was written?		*/
	unsigned int pt_mbz	: 1;		/* must be zero			*/
	unsigned int pt_global: 1;		/* should be zero in 586	*/
	unsigned int pt_avail : 3;		/* for programmer's use		*/
	unsigned int pt_base	: 20;		/* location of page?		*/
} pt_t;

extern int32 currpolicy;

// this is for frame which use for pd pt and pages/////////// 

typedef struct  _frame{
	int32 frame_id;
	int32 state;  // this frame is used or free
	int32 type;   // this frame is used for pd, pt or page
	struct _frame *next_frame; // pointer to the next frame;
} frame_t;

extern frame_t frame_tab[];

extern frame_t *frame_head;

//////////////////////////////////////////////////////////////

// this is for inverted page table///////////////////////////

typedef struct {
	int32 refcount; // ref count for pt
	pid32 pid;   // process id
	uint32 vpn;  // page number
} inverted_page;

extern inverted_page inverted_page_tab[];

/////////////////////////////////////////////////////////////

// this is for virtual addrees /////////////////////////////

typedef struct { 
	uint32 pg_offset    : 12;
	uint32 pt_offset    : 10;
	uint32 pd_offset    : 10;
} vd_t;

///////////////////////////////////////////////////////////

// this is for bs map//////////////////////////////////////
typedef struct {
	pid32 pid;
	uint32 vpn;  //starting page number
	uint32 npg;   // number of pages
	bsd_t bs_id; // store id
	bool8 allocated; //whether this map has been used;
} bs_map;

extern bs_map bs_map_tab[];


extern pt_t *global_pt[4];
extern pt_t *device_pt;
extern unsigned long tmp;

extern uint32 count_faults;

#define PAGEDIRSIZE	1024
#define PAGETABSIZE	1024

#define NBPG		4096	/* number of bytes per page	*/
#define FRAME0		1024	/* zero-th frame		*/

#define VPN0        4096    /* first start vpn*/

#ifndef NFRAMES
#define NFRAMES		500	/* number of frames		*/
#endif

#define MAP_SHARED 1
#define MAP_PRIVATE 2

#define FIFO 3
#define GCA 4

#define MAX_ID		7		/* You get 8 mappings, 0 - 7 */
#define MIN_ID		0


#define DEV_VPN 589824  // starting page number for device
#define DEV_PTN 576     // dev pt index

// for state
#define FRAME_FREE 0
#define FRAME_USED 1

// for type
#define FRAME_PD 0
#define FRAME_PT 1
#define FRAME_PG 2

// transformation function between vpn, vd, pd, framedid////

#define VPN_TO_VD(vpn) (uint32)((uint32)vpn * NBPG)
#define FID_TO_VD(fid) (uint32)(((uint32)fid+FRAME0) * NBPG)
#define VD_TO_VPN(vd)  (uint32)((uint32)vd / NBPG)
#define VD_TO_FID(vd)  (uint32)((uint32)vd / NBPG-FRAME0)
#define FID_TO_VPN(fid) (uint32)((uint32)fid+FRAME0)


// in /paging/page_enable.c
extern void set_cr0(unsigned long n);
extern void set_cr3(unsigned long n);
extern unsigned long read_cr0(void);
extern unsigned long read_cr2(void);
extern unsigned long read_cr3(void);
extern void enable_paging();
extern void set_pd_reg(unsigned long pd_reg);

// in /paging/page.c

extern pt_t* pt_allocate(void);
extern int32 init_global_pt(void);
extern pd_t* pd_allocate(void);


// in paging/frame.c
extern void frame_initial(void);
extern int32 get_free_frame_fifo(void);
extern int32 get_free_frame_gca(void);
extern int32 rm_frame_fifo(int32 frameid);
extern int32 free_frame(int32 frame_id);
extern int32 get_free_frame(void);
extern int32 frame_allocate(void);

// in paging/pg_fault_handler.c
extern void pg_fault_handler(void);
//extern uint32 get_faults(void);

// in paging/pgfault.s
extern void pgfault(void);

// in bs_map.c
extern bs_map* get_bs_map(pid32 pid, uint32 vpn);
extern void initial_bs_map_tab(void);
extern int32 add_bs_map(pid32 pid, uint32 vpn, uint32 npg, bsd_t bs_id);
extern int32 rm_bs_map(pid32 pid);
extern void checkfault();

#endif // __PAGING_H_
