/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/vaddr.h"
#include "threads/synch.h"

struct lock hash_lock;
struct list frame_table;

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();

	lock_init(&hash_lock);
	list_init(&frame_table);

#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) 
{

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* Check wheter the upage is already occupied or not. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: Create the page, fetch the initialier according to the VM type,
		 * TODO: and then create "uninit" page struct by calling uninit_new. You
		 * TODO: should modify the field after calling the uninit_new. */
		/* TODO: Insert the page into the spt. */

		// [HERE] 2

	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */

struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) 
{
	struct page page;	
	/* TODO: Fill this function. */
	// [HERE] 1

	page.va =pg_round_down(va); 

	struct hash_elem *e = hash_find(&spt->hash_table, &page.hash_elem);
	
	if (!e) return NULL;

	return hash_entry(e, struct page, hash_elem);
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt,
                 struct page *page)
{
    bool succ = false;

	lock_acquire(&hash_lock);

    if (hash_insert(&spt->hash_table, &page->hash_elem) == NULL)
    {
        succ = true;
    }

	lock_release(&hash_lock);

    return succ;
}


void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) 
{
	lock_acquire(&hash_lock);

	hash_delete(&spt->hash_table, &page->hash_elem);
	vm_dealloc_page (page);

	lock_release(&hash_lock);

	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) 
{
    struct frame *frame = NULL;

    /* TODO: Fill this function. */
    // [HERE] 2

    void *kva = palloc_get_page(PAL_USER);
    if(kva == NULL) {PANIC("kernel panic");
    } else {frame = malloc(sizeof(frame));
    }
    frame->kva = kva;

    ASSERT (frame != NULL);
    ASSERT (frame->page == NULL);

    return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) 
{
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */
	// [HERE] 2

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */

bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: Fill this function */
	// [HERE] 2
  
	page = spt_find_page(&thread_current()->spt, va);
    if (page == NULL)
        return false;


	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) 
{

	struct frame *frame = vm_get_frame();
	if (frame == NULL) return false;

	/* Set links */
	//물리 프레임에 가상 페이지를 연결
	frame->page = page;
	//가상 메모리에 물리 프레임 연결
	page->frame = frame;
	
	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	// [HERE] 2
	if (!pml4_set_page(thread_current()->pml4, page->va, frame->kva, page->writable))
    	return false;


	return swap_in (page, frame->kva);
}

uint64_t do_hash(const struct hash_elem *e, void *aux)
{
    // [HERE] 1
	struct page *p = hash_entry(e, struct page, hash_elem); 

	return hash_bytes(&p->va, sizeof(p->va));  
}

bool hash_less(const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
    // [HERE] 1
	struct page *pa = hash_entry(a, struct page, hash_elem);
	struct page *pb = hash_entry(b, struct page, hash_elem);

	return pa->va < pb->va;
}


/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) 
{
	// [HERE] 1

	hash_init(&spt->hash_table, do_hash, hash_less, NULL);
}


/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) 
{
	// [HERE] 3
}

void hash_kill(struct hash_elem *e, void *aux)
{
    // [HERE] 3
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) 
{
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	// [HERE] 3
}