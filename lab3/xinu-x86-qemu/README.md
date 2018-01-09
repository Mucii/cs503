# This is for XINU based paging and demand paging

## Paging
This project follows x86 4GB paging with first 1GB for stack and 3GB for private heap as below:

    ------------------------------------------------------------------

      device memory
      1024 pages starting at 0x90000000
      (pages 589824 - 590847)

      ------------------------------------------------------------------

      virtual heap
      (pages 4096 and beyond)

      ------------------------------------------------------------------

      3072 frames
      (pages 1024 - 4095)

      ------------------------------------------------------------------

      free memory
      (pages 327 - 1023)

      ------------------------------------------------------------------

      XINU text, data, bss
      (pages 257 - 326)

      ------------------------------------------------------------------

      Memory reserved for Boot loader
      (pages 0 - 256)

      ------------------------------------------------------------------


All memory below page 4096 will be “global” (or shared). That is, it is usable and visible by all processes and accessible by simply using physical addresses. In other words, all page tables for page entries 0-4095 implement an identity map. That is, the physical address is the virtual address.

Memory at page 4096 and above constitute a process’s private virtual memory. This private address space is visible only to the process that owns it.

## Page table and Page dir

We use two-layer Paging with Page dir and Page table. The last 20 digits of Page dir detemines Page table and last 20 digits of Paging table detemines Page. For each process it will have its own page dir in the memory through out the whole time. And will have its page table and pages in the memory on demamnd.

## Demand Paging

We use a remote disk (backstore) to store all pages at the beginning and only put it into memory when needed. We use two policy GCA and FIFO. Demand Paging is handled by IRS. 

We use a linked list to keep all pages put in memory for FIFO. When need to replace, we always remove the first page on the linked list. When need to remove, we always add the page to the last of the linked list.

We check dirty and refered digits in corresping page table for each page to determine pages replacement for GCA


## API 

vcreate() to create a process with virtual heap. vgetmem() and vfreemem() to alocate and dealocate memory for a process with virtual heap.

The construction of Paging is mainly in /paging

pgfault.s IRS for paging, note there is not cli/sti to make writting consistent with reading

page_fault_handler.c page fault handler for paging, I use semaphore for the handler to make sure only one can access the global data.

frame.c frame mechanics for paging, including FIFO and GCA algorithm for frame evicting.

page.c page mechanics and paging initialization 

bs_map.c backstore mapping 

see all global definition in /include/paging.h
