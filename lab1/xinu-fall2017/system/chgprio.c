/* chgprio.c - chgprio*/

#include <xinu.h>

/*----------------------------------------------------------------------------
 *   chgprio - Change the priority of groups
 *----------------------------------------------------------------------------
 */

void chgprio(
	int group, 
	pri16 newprio
	)
{
	intmask mask;


	mask=disable();
	if(group==PROPORTIONALSHARE){
		psprioin=newprio;
		psprio=newprio;
	}else if(group==TSSCHED){
		tsprioin=newprio;
		tsprio=newprio;
	}


	restore(mask);
	return ;
}
