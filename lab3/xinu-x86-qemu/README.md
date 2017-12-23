# This is for XINU based paging and demand paging

## see all in the paging folder

pgfault.s IRS for paging, note there is not cli/sti to make writting consistent with reading

page_fault_handler.c page fault handler for paging, I use semaphore for the handler to make sure only one can access the global data.

frame.c frame mechanics for paging, including FIFO and GCA algorithm for frame evicting.

page.c page mechanics and paging initialization 

bs_map.c backstore mapping 

## see all global definition in /include/paging.h
