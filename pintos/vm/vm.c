/* vm.c: 가상 메모리 객체를 위한 범용(Generic) 인터페이스. */

#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"
<<<<<<< HEAD
/* 각 하위 서브시스템의 초기화 코드를 호출하여
 * 가상 메모리 서브시스템을 초기화한다. */
=======
#include "threads/vaddr.h"
#include "threads/synch.h"

// Project3
struct lock hash_lock;
struct list frame_table;

/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
>>>>>>> main
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
<<<<<<< HEAD
#ifdef EFILESYS  /* 프로젝트 4용 */
=======

	lock_init(&hash_lock);
	list_init(&frame_table);

#ifdef EFILESYS  /* For project 4 */
>>>>>>> main
	pagecache_init ();
#endif
	register_inspect_intr ();
	/* 위 줄들은 수정하지 말 것. */
	/* TODO: 여기에 코드를 작성하세요. */
}

/* 페이지의 타입을 얻는다. 이 함수는 페이지가 초기화된 후
 * 타입을 알고 싶을 때 유용하다.
 * 현재 이 함수는 완전히 구현되어 있다. */
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

/* 헬퍼들 */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);

/* 초기화 함수를 동반한 보류 중(pending) 페이지 객체를 만든다.
 * 페이지를 만들고 싶다면 직접 만들지 말고 이 함수 또는
 * `vm_alloc_page`를 통해 만들어라. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) 
{

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;

	/* 주어진 upage가 이미 점유되었는지 확인. */
	if (spt_find_page (spt, upage) == NULL) {
		/* TODO: 페이지를 생성하고, VM 타입에 따라 초기화 함수를 가져온 뒤,
		 * TODO: uninit_new를 호출하여 "uninit" 페이지 구조체를 생성한다.
		 * TODO: uninit_new 호출 후 필드를 적절히 수정할 것. */
		/* TODO: 페이지를 spt에 삽입. */

		// [HERE] 2

	}
err:
	return false;
}

<<<<<<< HEAD
/* spt에서 VA를 찾아 페이지를 반환. 실패 시 NULL 반환. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) 
{
	struct page *page = NULL;
	/* TODO: 이 함수를 완성하세요. */
=======
/* Find VA from spt and return page. On error, return NULL. */
// “가상 주소 va에 해당하는 page 구조체를 supplemental page table에서 찾아 리턴한다”
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) 
{
	struct page page;	// 걍 va 탐색용 임시 페이지임
	/* TODO: Fill this function. */
>>>>>>> main
	// [HERE] 1

	page.va =pg_round_down(va); // 페이지 단위로 정렬

	struct hash_elem *e = hash_find(&spt->hash_table, &page.hash_elem);
	
	if (!e) return NULL;
	
	return hash_entry(e, struct page, hash_elem);
}

/* 검증과 함께 PAGE를 spt에 삽입. */
bool
<<<<<<< HEAD
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	int succ = false;
	/* TODO: 이 함수를 완성하세요. */
	// [HERE] 1
=======
spt_insert_page (struct supplemental_page_table *spt,
                 struct page *page)
{
    bool succ = false;
>>>>>>> main

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

/* 축출될(frame eviction) 프레임을 반환. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: 축출 정책(eviction policy)은 자유롭게 구현. */

	return victim;
}

/* 한 페이지를 축출하고 해당 프레임을 반환.
 * 실패 시 NULL 반환. */
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: victim을 스왑 아웃하고 축출된 프레임을 반환. */

	return NULL;
}

/* palloc()으로 프레임을 얻는다. 사용 가능한 페이지가 없으면
 * 페이지를 축출한 뒤 반환한다. 이 함수는 항상 유효한 주소를 반환한다.
 * 즉, 유저 풀 메모리가 가득 차면 가용 메모리 공간을 얻기 위해
 * 프레임을 축출한다. */
static struct frame *
vm_get_frame (void) 
{
	struct frame *frame = NULL;
	/* TODO: 이 함수를 완성하세요. */
	// [HERE] 2

	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* 스택 성장 처리. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* 쓰기 보호된 페이지의 fault 처리 */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* 성공 시 true 반환 */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) 
{
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: fault를 검증 */
	/* TODO: 여기에 코드를 작성하세요 */
	// [HERE] 2

	return vm_do_claim_page (page);
}

/* 페이지 해제.
 * 이 함수는 수정하지 말 것. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* VA에 할당된 페이지를 클레임(claim)한다. */
bool
vm_claim_page (void *va UNUSED) {
	struct page *page = NULL;
	/* TODO: 이 함수를 완성하세요 */
	// [HERE] 2

	return vm_do_claim_page (page);
}

/* PAGE를 클레임하고 MMU를 설정한다. */
static bool
vm_do_claim_page (struct page *page) 
{
	struct frame *frame = vm_get_frame ();

	/* 연결 설정 */
	frame->page = page;
	page->frame = frame;

	/* 페이지의 VA를 프레임의 PA에 매핑하도록 페이지 테이블 엔트리를 삽입. */
	// [HERE] 2

	return swap_in (page, frame->kva);
}

<<<<<<< HEAD
/* 새로운 보조 페이지 테이블(supplemental page table) 초기화 */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) 
{
	// [HERE] 1
	/* 컨테이너 준비 */
	struct hash_elem *e, *a, *b;
	void *aux; 
	bool hash_succ;
	
	hash_succ = hash_init(&spt->hash_table, do_hash(e,aux), hash_less(a,b,aux), NULL);
	if(!hash_succ) return -1;


}

=======
>>>>>>> main
uint64_t do_hash(const struct hash_elem *e, void *aux)
{
    // [HERE] 1
	struct page *p = hash_entry(e, struct page, hash_elem);  // hash_elem -> struct page
	return hash_bytes(&p->va, sizeof(p->va));  // va를 기준으로 해시값 생성
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


/* src에서 dst로 보조 페이지 테이블을 복사 */
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

/* 보조 페이지 테이블이 보유한 자원을 해제 */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) 
{
	/* TODO: 스레드가 보유한 모든 보조 페이지 테이블을 파괴하고,
	 * TODO: 수정된 내용은 스토리지에 모두 반영(writeback)할 것. */
	// [HERE] 3
}
