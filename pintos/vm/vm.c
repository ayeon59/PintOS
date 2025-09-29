/* vm.c: Generic interface for virtual memory objects. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
#include "threads/vaddr.h"

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
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
bool vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) 
{
    ASSERT(VM_TYPE(type) != VM_UNINIT)

    struct supplemental_page_table *spt = &thread_current()->spt;

    if (spt_find_page(spt, upage) == NULL)
    {
        struct page *new_page = malloc(sizeof(struct page));
        if (new_page == NULL)
        {
            goto err;
        }

        bool (*page_initializer)(struct page *, enum vm_type, void *);

        switch (VM_TYPE(type))
        {
            case VM_ANON:
                page_initializer = anon_initializer;
                break;
            case VM_FILE:
                page_initializer = file_backed_initializer;
                break;
            default:
                goto err;
        }

        uninit_new(new_page, upage, init, type, aux, page_initializer);

        new_page->writable = writable;

        if (!spt_insert_page(spt, new_page))
        {
            free(new_page);
            goto err;
        }

        return true;
    }
err:
    return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) 
{
	struct page page;
    struct hash_elem *e;

    page.va = pg_round_down(va);
    e = hash_find(&spt->hash_table, &page.hash_elem);

    if (e == NULL)
    {
        return NULL;
    }
    return hash_entry(e, struct page, hash_elem);
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	/* TODO: Fill this function. */
	bool succ = false;
    if (hash_insert(&spt->hash_table, &page->hash_elem) == NULL)
    {
        succ = true;
    }
    return succ;

}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
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
vm_get_frame (void) {
	/* TODO: Fill this function. */
	struct frame *frame = NULL;
    void *kva = palloc_get_page(PAL_USER);
    if (kva == NULL)
    {
        PANIC("what should i do");
    }

    frame = malloc(sizeof(struct frame));
    frame->kva = kva;
    frame->page = NULL;

    ASSERT(frame != NULL);
    ASSERT(frame->page == NULL);
    return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) 
{
	 vm_alloc_page(VM_ANON | VM_MARKER_0, pg_round_down(addr), true);
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) 
{
}

enum sg_type
{
    GENERAL,
    PUSH
};

static bool is_stack_pointer_valid(void *rsp, enum sg_type type)
{
    switch (type)
    {
        case GENERAL:
            return rsp >= USER_STACK_END;
        case PUSH:
            return rsp - 8 >= USER_STACK_END;
        default:
            PANIC("invalid stack growth type");
    }
}
static bool is_addr_access_valid(void *rsp, void *addr, enum sg_type type)
{
    switch (type)
    {
        case GENERAL:
            return addr >= rsp;
        case PUSH:
            return addr == rsp - 8;
        default:
            PANIC("invalid stack growth type");
    }
}

static bool is_addr_valid(void *addr)
{
    return addr <= USER_STACK;
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

    void *rsp = f->rsp;

    if (addr == NULL)
    {
        return false;
    }

    if (is_kernel_vaddr(addr))
    {
        return false;
    }

    if (not_present)
    {
        if (!user)
        {
            /* kenel 모드에서 page fault 발생 시, 이전에 syscall에서 설정해
             * 두었던 rsp 값을유효한 rsp 값으로 설정 */
            rsp = thread_current()->rsp;
        }

        if (is_stack_pointer_valid(rsp, GENERAL) &&
            is_addr_access_valid(rsp, addr, GENERAL) && is_addr_valid(addr))
        {
            /* 일반적인 stack 확장 경우. - stack growth signal로 판단한다 */
            vm_stack_growth(addr);
        }
        else if (is_stack_pointer_valid(rsp, PUSH) &&
                 is_addr_access_valid(rsp, addr, PUSH) && is_addr_valid(addr))
        {
            /* stack의 push 동작 시 - stack growth signal로 판단한다 */
            vm_stack_growth(addr);
        }

        page = spt_find_page(spt, addr);
        if (page == NULL)
        {
            /* spt에 페이지가 존재하지 않으면 처리 실패. */
            return false;
        }

        if (write == true && page->writable == false)
        {
            /* spt에 존재하는 page가 writable하지 않으면 처리 실패. */
            return false;
        }

        return vm_do_claim_page(page);
    }
    else
    {
        return false;
    }
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
	/* TODO: Fill this function */

	struct thread *curr = thread_current();
    struct page *page = spt_find_page(&curr->spt, va);

    if (page == NULL)
    {
        return false;
    }

    return vm_do_claim_page(page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();

	if (frame == NULL)
    {
        return false;
    }

	/* Set links */
	frame->page = page;
	page->frame = frame;

	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	struct thread *curr = thread_current();
    if (!pml4_set_page(curr->pml4, page->va, frame->kva, page->writable))
    {
        free(frame);
        return false;
    }

    if (!swap_in(page, frame->kva))
    {
        free(frame);
        pml4_clear_page(curr->pml4, page->va);
        return false;
    }

    return true;

}

uint64_t do_hash(const struct hash_elem *e, void *aux)
{
    struct page *p = hash_entry(e, struct page, hash_elem);
    uintptr_t key = (uintptr_t) p->va;
    return hash_bytes(&key, sizeof(p->va));
}

bool hash_less(const struct hash_elem *a, const struct hash_elem *b, void *aux)
{
    struct page *page_a = hash_entry(a, struct page, hash_elem);
    struct page *page_b = hash_entry(b, struct page, hash_elem);

    return (uintptr_t) page_a->va < (uintptr_t) page_b->va;
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED)
 {
	hash_init(&spt->hash_table, do_hash, hash_less, NULL);
}


/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) 
{
	struct hash_iterator i;

    hash_first(&i, &src->hash_table);
    while (hash_next(&i))
    {
        struct page *parent_page =
            hash_entry(hash_cur(&i), struct page, hash_elem);
        enum vm_type future_type = page_get_type(
            parent_page); /* uninit 페이지가 미래에 될 타입을 가져온다. */
        void *upage = parent_page->va;
        bool writable = parent_page->writable;

        if (VM_TYPE(parent_page->operations->type) == VM_UNINIT)
        {
            struct lazy_load_info *aux =
                (struct lazy_load_info *) malloc(sizeof(struct lazy_load_info));
            memcpy(aux, parent_page->uninit.aux, sizeof(struct lazy_load_info));

            if (!vm_alloc_page_with_initializer(future_type, upage, writable,
                                                parent_page->uninit.init, aux))
            {
                return false;
            }
        }
        else
        {
            if (!vm_alloc_page(future_type, upage, writable))
            {
                return false;
            }

            if (!vm_claim_page(upage))
            {
                return false;
            }

            struct page *child_page = spt_find_page(dst, upage);
            memcpy(child_page->frame->kva, parent_page->frame->kva, PGSIZE);
            child_page->frame->page = child_page;
        }
    }

    return true;
}

void hash_destructor(struct hash_elem *e, void *aux)
{
    struct page *page_to_destroy = hash_entry(e, struct page, hash_elem);
    destroy(page_to_destroy); 
    free(page_to_destroy);  
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) 
{
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	hash_destroy(&spt->hash_table, hash_destructor);
}
