26a27
> #include <linux/memremap.h>
47a49
> static DEFINE_PER_CPU(struct pagevec, lru_deactivate_pvecs);
92,272c94
< /**
<  * Two special cases here: we could avoid taking compound_lock_irqsave
<  * and could skip the tail refcounting(in _mapcount).
<  *
<  * 1. Hugetlbfs page:
<  *
<  *    PageHeadHuge will remain true until the compound page
<  *    is released and enters the buddy allocator, and it could
<  *    not be split by __split_huge_page_refcount().
<  *
<  *    So if we see PageHeadHuge set, and we have the tail page pin,
<  *    then we could safely put head page.
<  *

<                */
<               if (put_page_testzero(page))
<                       __put_single_page(page);
< }
<
< static __always_inline
< void put_refcounted_compound_page(struct page *page_head, struct page *page)
< {
<       if (likely(page != page_head && get_page_unless_zero(page_head))) {
<               unsigned long flags;
<
<               /*
<                * @page_head wasn't a dangling pointer but it may not
<                * be a head page anymore by the time we obtain the
<                * lock. That is ok as long as it can't be freed from
<                * under us.
<                */
<               flags = compound_lock_irqsave(page_head);
<               if (unlikely(!PageTail(page))) {
<                       /* __split_huge_page_refcount run before us */
<                       compound_unlock_irqrestore(page_head, flags);
<                       if (put_page_testzero(page_head)) {
<                               /*
<                                * The @page_head may have been freed
<                                * and reallocated as a compound page
<                                * of smaller order and then freed
<                                * again.  All we know is that it
<                                * cannot have become: a THP page, a
<                                * compound page of higher order, a
<                                * tail page.  That is because we
<                                * still hold the refcount of the
<                                * split THP tail and page_head was
<                                * the THP head before the split.
<                                */
<                               if (PageHead(page_head))
<                                       __put_compound_page(page_head);
<                               else
<                                       __put_single_page(page_head);
<                       }
< out_put_single:
<                       if (put_page_testzero(page))
<                               __put_single_page(page);
<                       return;
<               }
<               VM_BUG_ON_PAGE(page_head != compound_head(page), page);
<               /*
<                * We can release the refcount taken by
<                * get_page_unless_zero() now that
<                * __split_huge_page_refcount() is blocked on the
<                * compound_lock.
<                */
<               if (put_page_testzero(page_head))
<                       VM_BUG_ON_PAGE(1, page_head);
<               /* __split_huge_page_refcount will wait now */
<               VM_BUG_ON_PAGE(page_mapcount(page) <= 0, page);
<               atomic_dec(&page->_mapcount);
<               VM_BUG_ON_PAGE(atomic_read(&pa
<       /* Ref to put_compound_page() comment. */
<       if (!__compound_tail_refcounted(page_head)) {
<               smp_rmb();
<               if (likely(PageTail(page))) {
<                       /*
<                        * This is a hugetlbfs page or a slab
<                        * page. __split_huge_page_refcount
<                        * cannot race here.
<                        */
<                       VM_BUG_ON_PAGE(!PageHead(page_head), page_head);
<                       __get_page_tail_foll(page, true);
<                       return true;
<               } else {
<                       /*
<                        * __split_huge_page_refcount run
<                        * before us, "page" was a THP
<                        * tail. The split page_head has been
<                        * freed and reallocated as slab or
<                        * hugetlbfs page of smaller order
<                        * (only possible if reallocated as
<                        * slab on x86).
<                        */
<                       return false;
<               }
<       }
<
<       got = false;
<       if (likely(page != page_head && get_page_unless_zero(page_head))) {
<               /*
<                * page_head wasn't a dangling pointer but it
<                * may not be a head page anymore by the time
<                * we obtain the lock. That is ok as long as it
<                * can't be freed from under us.
<                */
<               flags = compound_lock_irqsave(page_head);
<               /* here __split_huge_page_refcount won't run anymore */
<               if (likely(PageTail(page))) {
<                       __get_page_tail_foll(page, false);
<                       got = true;
<               }
<               compound_unlock_irqrestore(page_head, flags);
<               if (unlikely(!got))
<                       put_page(page_head);
<       }
<       return got;
< }
< EXPORT_SYMBOL(__get_page_tail);
---
> EXPORT_SYMBOL(__put_page);
606a363
>       page = compound_head(page);
801a559,576
>
> static void lru_deactivate_fn(struct page *page, struct lruvec *lruvec,
>                           void *arg)
> {
>       if (PageLRU(page) && PageActive(page) && !PageUnevictable(pag
>
>               __count_vm_event(PGDEACTIVATE);
>               update_page_reclaim_stat(lruvec, file, 0);
>       }
> }
>
827a603,606
>       pvec = &per_cpu(lru_deactivate_pvecs, cpu);
>       if (pagevec_count(pvec))
>               pagevec_lru_move_fn(pvec, lru_deactivate_fn, NULL);
>
856a636,655
> /**
>  * deactivate_page - deactivate a page
>  * @page: page to deactivate
>  *
>  * deactivate_page() moves @page to the inactive list if @page was on the active
>  * list and was not an unevictable page.  This is done to accelerate the reclaim
>  * of @page.
>  */
> void deactivate_page(struct page *page)
> {
>       if (PageLRU(page) && PageActive(page) && !PageUnevictable(page)) {
>               struct pagevec *pvec = &get_cpu_var(lru_deactivate_pvecs);
>
>               page_cache_get(page);
>               if (!pagevec_add(pvec, page))
>                       pagevec_lru_move_fn(pvec, lru_deactivate_fn, NULL);
>               put_cpu_var(lru_deactivate_pvecs);
>       }
> }
>
885a685
>                   pagevec_count(&per_cpu(lru_deactivate_pvecs, cpu)) ||
921,929d720
<               if (unlikely(PageCompound(page))) {
<                       if (zone) {
<                               spin_unlock_irqrestore(&zone->lru_lock, flags);
<                               zone = NULL;
<                       }
<                       put_compound_page(page);
<                       continue;
<               }
<
939a731
>               page = compound_head(page);
941a734,742
>
>               if (PageCompound(page)) {
>                       if (zone) {
>                               spin_unlock_irqrestore(&zone->lru_lock, flags);
>                               zone = NULL;
>                       }
>                       __put_compound_page(page);
>                       continue;
>               }
