#ifndef MM_ADDRESS_H
#define MM_ADDRESS_H

#define ENTRY_DIR_PAGES       0

#define TOTAL_PAGES 1024
#define NUM_PAG_KERNEL 256
#define PAG_LOG_INIT_CODE (PAG_LOG_INIT_DATA+NUM_PAG_DATA)
#define FRAME_INIT_CODE (PH_USER_START>>12)
#define NUM_PAG_CODE 8
#define PAG_LOG_INIT_DATA (L_USER_START>>12)
#define NUM_PAG_DATA 20 // Interpreto que es data + stack
#define PAGE_SIZE 0x1000

/* Memory distribution */
/***********************/


#define KERNEL_START     0x10000
#define L_USER_START        0x100000
#define USER_ESP	L_USER_START+(NUM_PAG_DATA)*0x1000-16

#define USER_FIRST_PAGE	(L_USER_START>>12)

#define PH_PAGE(x) (x>>12)

#endif
