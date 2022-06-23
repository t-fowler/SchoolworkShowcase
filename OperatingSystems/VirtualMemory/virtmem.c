/*
 * Skeleton code for CSC 360, Spring 2021,  Assignment #4
 *
 * Prepared by: Michael Zastre (University of Victoria) 2021
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/queue.h>
#include <unistd.h>

/*
 * Some compile-time constants.
 */

#define REPLACE_NONE 0
#define REPLACE_FIFO 1
#define REPLACE_LRU  2
#define REPLACE_SECONDCHANCE 3
#define REPLACE_OPTIMAL 4


#define TRUE 1
#define FALSE 0
#define PROGRESS_BAR_WIDTH 60
#define MAX_LINE_LEN 100
#define MAX_PAGES 1024


/*
 * Some function prototypes to keep the compiler happy.
 */
int setup(void);
int teardown(void);
int output_report(void);
long resolve_address(long, int);
void error_resolve_address(long, int);
void fifo_insert(long);
long fifo_replace();
void lru_insert();
void lru_access(long);
long lru_replace();
long secondchance_replace();
void swap_in(long, long, int);
void swap_out(long);

/*
 * Variables used to keep track of the number of memory-system events
 * that are simulated.
 */
int page_faults = 0;
int mem_refs    = 0;
int swap_outs   = 0;
int swap_ins    = 0;


/*
 * Page-table information. You may want to modify this in order to
 * implement schemes such as SECONDCHANCE. However, you are not required
 * to do so.
 */
struct page_table_entry *page_table = NULL;
struct page_table_entry {
    long page_num;
    long frame_num;                             /* Set when table entries are added to a queue. */
    int dirty;
    int free;
    int referenced;                             /* Set when a frame is accessed. */
    STAILQ_ENTRY(page_table_entry) s_entries;   /* Single linked List for FIFO/Secondchance */
    TAILQ_ENTRY(page_table_entry) t_entries;    /* LRU uses a doubly linked list for the faster removal. */
};


/*
 * These global variables will be set in the main() function. The default
 * values here are non-sensical, but it is safer to zero out a variable
 * rather than trust to random data that might be stored in it -- this
 * helps with debugging (i.e., eliminates a possible source of randomness
 * in misbehaving programs).
 */

int size_of_frame = 0;  /* power of 2 */
int size_of_memory = 0; /* number of frames */
int page_replacement_scheme = REPLACE_NONE;


/********************************************************************************************
 *              This section implements a number of different types of queues               *
 *          used for implementing the FIFO, LRU and Secondchance replacement policies.      *
 ********************************************************************************************/
STAILQ_HEAD(fifo_queue_t, page_table_entry) fifo_queue;
TAILQ_HEAD(lru_queue_t, page_table_entry) lru_queue;


/********************************************************
 *    Some functions for implementing the FIFO queue.   *
 ********************************************************/

/* Inserts frame_num to the back of the queue. */
void fifo_insert(long frame_num) {
    struct page_table_entry *new_entry = page_table + frame_num;
    STAILQ_INSERT_TAIL(&fifo_queue, new_entry, s_entries);
}


/* Function to implement fifo page replacement policy. */
long fifo_replace() {
    struct page_table_entry *head = STAILQ_FIRST(&fifo_queue);
    long victim_frame;
    
    if (head == NULL) {
        fprintf(stderr, "%s\n", "Simulation error: fifo_replace attempted on an empty queue.");
        exit(1);
    }

    victim_frame = head->frame_num;
    STAILQ_REMOVE_HEAD(&fifo_queue, s_entries);
    return victim_frame;
}


/********************************************************
 *    Some functions for implementing the LRU queue.    *
 ********************************************************/

/* Insert a new frame to the tail of the queue. */
void lru_insert(long frame_num) {
    struct page_table_entry *new_entry = page_table + frame_num;
    TAILQ_INSERT_TAIL(&lru_queue, new_entry, t_entries);
}

/* The accessed_frame is removed and replaced onto the back of the queue. */
void lru_access(long accessed_frame) {
    struct page_table_entry *f = page_table + accessed_frame;
    
    if (f == NULL) {
        fprintf(stderr, "%s\n", "Simulation error: lru_access used when accessed_frame is not in the queue.");
        exit(1);
    }
    
    TAILQ_REMOVE(&lru_queue, f, t_entries);          /* Remove the accessed frame from the queue and */
    TAILQ_INSERT_TAIL(&lru_queue, f, t_entries);     /* Replace it at the tail of the queue. */
}


/**
 * Function to implement least recently used page replacement policy.
 * Removes the victim frame from the queue. Need to swap in the new frame.
 */
long lru_replace() {
    struct page_table_entry *head = TAILQ_FIRST(&lru_queue);
    long victim_frame;

    if (head == NULL) {
        fprintf(stderr, "%s\n", "Simulation error: lru_replace attempted to delete from an empty queue.");;
        exit(1);
    }

    victim_frame = head->frame_num;
    TAILQ_REMOVE(&lru_queue, head, t_entries);

    return victim_frame;
}


/********************************************************************************
 *    Some functions for implementing the Secondchance queue. This replacement  *
 *  scheme uses the same queue type as the FIFO policy.                         *
 ********************************************************************************/

/* This function implements the second chance replacement policy. */
long secondchance_replace() {
    struct page_table_entry *head = STAILQ_FIRST(&fifo_queue);
    struct page_table_entry *tmp;
    long victim_frame;

    if (head == NULL) {
        fprintf(stderr, "%s\n", "Simulation error: secondchance_replace attempted to delete from an empty queue.");;
        exit(1);
    }

    while (head->referenced == 1) {
        head->referenced = 0;                               /* Reset referenced flag. */
        tmp = STAILQ_NEXT(head, s_entries);                 /* Grab the next head. Might be NULL if queue is only 1 long. */
        STAILQ_REMOVE_HEAD(&fifo_queue, s_entries);         /* Remove and re-insert previous head. */
        STAILQ_INSERT_TAIL(&fifo_queue, head, s_entries);
        if (tmp != NULL) head = tmp;                        /* Update to point at the new head. */
    }

   victim_frame = head->frame_num;                          /* Remove the victim frame. */
   STAILQ_REMOVE_HEAD(&fifo_queue, s_entries);
   
   return victim_frame;
}


/****************************************************************
 *    Some functions for swapping in/out page table entries.    *
 ****************************************************************/

/* This function swaps a page into physical memory. */
void swap_in(long page_num, long frame_num, int memwrite) {
    swap_ins++;

    /* Update the page table. */
    page_table[frame_num].page_num = page_num;
    page_table[frame_num].frame_num = frame_num;
    page_table[frame_num].free = FALSE;
    page_table[frame_num].referenced = 1;
    if (memwrite == TRUE) page_table[frame_num].dirty = TRUE;
    else                  page_table[frame_num].dirty = FALSE;

    /* Insert frame into the queue. */
    switch(page_replacement_scheme) {
        case REPLACE_FIFO:
        case REPLACE_SECONDCHANCE:
            fifo_insert(frame_num);
            break;
        
        case REPLACE_LRU:
            lru_insert(frame_num);
            break;
        
    }
}


/**
 * This function swaps out a page from physical memory. If the page
 * has been modified, it will update the swap_outs variable.
 */
void swap_out(long frame_num) {
    if (page_table[frame_num].dirty == TRUE) {
        swap_outs++;
    }
}


/*
 * Function to convert a logical address into its corresponding 
 * physical address. The value returned by this function is the
 * physical address (or -1 if no physical address can exist for
 * the logical address given the current page-allocation state.
 */

long resolve_address(long logical, int memwrite)
{
    int i;
    long page, frame;
    long offset;
    long mask = 0;
    long effective;

    /* Get the page and offset */
    page = (logical >> size_of_frame);

    for (i=0; i<size_of_frame; i++) {
        mask = mask << 1;
        mask |= 1;
    }
    offset = logical & mask;

    /* Find page in the inverted page table. */
    frame = -1;
    for ( i = 0; i < size_of_memory; i++ ) {
        if (!page_table[i].free && page_table[i].page_num == page) {
            frame = i;
            break;
        }
    }

    /* If frame is not -1, then we can successfully resolve the
     * address and return the result. */
    if (frame != -1) {
        switch (page_replacement_scheme) {                      /* Update page table entries upon accessing frame. */
            case REPLACE_FIFO:
            case REPLACE_SECONDCHANCE:
                page_table[frame].referenced = 1;
                break;

            case REPLACE_LRU:
                lru_access(frame);
                break;
        }

        if (memwrite == TRUE) page_table[frame].dirty = TRUE;   /* Update dirty bit if instruction is a memory write. */
        effective = (frame << size_of_frame) | offset;
        return effective;
    }


    /* If we reach this point, there was a page fault. Find
     * a free frame. */
    page_faults++;

    for ( i = 0; i < size_of_memory; i++) {
        if (page_table[i].free) {
            frame = i;
            break;
        }
    }


    /* If we found a free frame, then patch up the
     * page table entry and compute the effective
     * address. Otherwise choose a replacement policy. */
    if (frame != -1) {
        swap_in(page, frame, memwrite);                     /* swap_in provides behavior for updating a frame with a new page. */
        effective = (frame << size_of_frame) | offset;
        return effective;
    }
        switch (page_replacement_scheme) {                  /* Choose replacement policy. */
            case REPLACE_FIFO:
            frame = fifo_replace();
            break;

            case REPLACE_LRU:
            frame = lru_replace();
            break;

            case REPLACE_SECONDCHANCE:
            frame = secondchance_replace();
            break;
        }
        swap_out(frame);                                    /* Updates swap_outs if dirty bit is set. */
        swap_in(page, frame, memwrite);                     /* Updates a frame with a new page. */
        effective = (frame << size_of_frame) | offset;
        return effective;
}



/*
 * Super-simple progress bar.
 */
void display_progress(int percent)
{
    int to_date = PROGRESS_BAR_WIDTH * percent / 100;
    static int last_to_date = 0;
    int i;

    if (last_to_date < to_date) {
        last_to_date = to_date;
    } else {
        return;
    }

    printf("Progress [");
    for (i=0; i<to_date; i++) {
        printf(".");
    }
    for (; i<PROGRESS_BAR_WIDTH; i++) {
        printf(" ");
    }
    printf("] %3d%%", percent);
    printf("\r");
    fflush(stdout);
}


int setup()
{
    int i;

    page_table = (struct page_table_entry *)malloc(
        sizeof(struct page_table_entry) * size_of_memory
    );

    if (page_table == NULL) {
        fprintf(stderr,
            "Simulator error: cannot allocate memory for page table.\n");
        exit(1);
    }

    for (i=0; i<size_of_memory; i++) {
        page_table[i].free = TRUE;
    }

    /* Initialize queues. */
    switch (page_replacement_scheme) {  
        case REPLACE_FIFO:
        case REPLACE_SECONDCHANCE:
            STAILQ_INIT(&fifo_queue);
            break;

        case REPLACE_LRU:
            TAILQ_INIT(&lru_queue);
            break;
    }

    return -1;
}


int teardown()
{
    free(page_table);
    return -1;
}


void error_resolve_address(long a, int l)
{
    fprintf(stderr, "\n");
    fprintf(stderr, 
        "Simulator error: cannot resolve address 0x%lx at line %d\n",
        a, l
    );
    exit(1);
}


int output_report()
{
    printf("\n");
    printf("Memory references: %d\n", mem_refs);
    printf("Page faults: %d\n", page_faults);
    printf("Swap ins: %d\n", swap_ins);
    printf("Swap outs: %d\n", swap_outs);

    return -1;
}


int main(int argc, char **argv)
{
    /* For working with command-line arguments. */
    int i;
    char *s;

    /* For working with input file. */
    FILE *infile = NULL;
    char *infile_name = NULL;
    struct stat infile_stat;
    int  line_num = 0;
    int infile_size = 0;

    /* For processing each individual line in the input file. */
    char buffer[MAX_LINE_LEN];
    long addr;
    char addr_type;
    int  is_write;

    /* For making visible the work being done by the simulator. */
    int show_progress = FALSE;

    /* Process the command-line parameters. Note that the
     * REPLACE_OPTIMAL scheme is not required for A#3.
     */
    for (i=1; i < argc; i++) {
        if (strncmp(argv[i], "--replace=", 9) == 0) {
            s = strstr(argv[i], "=") + 1;
            if (strcmp(s, "fifo") == 0) {
                page_replacement_scheme = REPLACE_FIFO;
            } else if (strcmp(s, "lru") == 0) {
                page_replacement_scheme = REPLACE_LRU;
            } else if (strcmp(s, "secondchance") == 0) {
                page_replacement_scheme = REPLACE_SECONDCHANCE;
            } else if (strcmp(s, "optimal") == 0) {
                page_replacement_scheme = REPLACE_OPTIMAL;
            } else {
                page_replacement_scheme = REPLACE_NONE;
            }
        } else if (strncmp(argv[i], "--file=", 7) == 0) {
            infile_name = strstr(argv[i], "=") + 1;
        } else if (strncmp(argv[i], "--framesize=", 12) == 0) {
            s = strstr(argv[i], "=") + 1;
            size_of_frame = atoi(s);
        } else if (strncmp(argv[i], "--numframes=", 12) == 0) {
            s = strstr(argv[i], "=") + 1;
            size_of_memory = atoi(s);
        } else if (strcmp(argv[i], "--progress") == 0) {
            show_progress = TRUE;
        }
    }

    if (infile_name == NULL) {
        infile = stdin;
    } else if (stat(infile_name, &infile_stat) == 0) {
        infile_size = (int)(infile_stat.st_size);
        /* If this fails, infile will be null */
        infile = fopen(infile_name, "r");  
    }


    if (page_replacement_scheme == REPLACE_NONE ||
        size_of_frame <= 0 ||
        size_of_memory <= 0 ||
        infile == NULL)
    {
        fprintf(stderr, 
            "usage: %s --framesize=<m> --numframes=<n>", argv[0]);
        fprintf(stderr, 
            " --replace={fifo|lru|optimal} [--file=<filename>]\n");
        exit(1);
    }


    setup();

    while (fgets(buffer, MAX_LINE_LEN-1, infile)) {
        line_num++;
        if (strstr(buffer, ":")) {
            sscanf(buffer, "%c: %lx", &addr_type, &addr);
            if (addr_type == 'W') {
                is_write = TRUE;
            } else {
                is_write = FALSE;
            }

            if (resolve_address(addr, is_write) == -1) {
                error_resolve_address(addr, line_num);
            }
            mem_refs++;
        } 

        if (show_progress) {
            display_progress(ftell(infile) * 100 / infile_size);
        }
    }
    

    teardown();
    output_report();

    fclose(infile);

    exit(0);
}
