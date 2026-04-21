168,169d167
<               struct list_head *lh;
<
191,192c189
<               lh = se->list.next;
<               se = list_entry(lh, struct swap_extent, list);
---
>               se = list_next_entry(se, list);
791,793d787
<       if (!count)
<               mem_cgroup_uncharge_swap(entry);
<
798a793
>               mem_cgroup_uncharge_swap(entry);
906c901
<               page = list_entry(page->lru.next, struct page, lru);
---
>               page = list_next_entry(page, lru);
931a927,929
>       /* The page is part of THP and cannot be reused */
>       if (PageTransCompound(page))
>               return 0;
1011c1009
<                               (!page_mapped(page) || vm_swap_full())) {
---
>                   (!page_mapped(page) || mem_cgroup_swap_full(page))) {
1114c1112
< static inline int maybe_same_pte(pte_t pte, pte_t swp_pte)
---
> static inline int pte_same_as_swp(pte_t pte, pte_t swp_pte)
1116,1126c1114
< #ifdef CONFIG_MEM_SOFT_DIRTY
<       /*
<        * When pte keeps soft dirty bit the pte generated
<        * from swap entry does not has it, still it's same
<        * pte from logical point of view.
<        */
<       pte_t swp_pte_dirty = pte_swp_mksoft_dirty(swp_pte);
<       return pte_same(pte, swp_pte) || pte_same(pte, swp_pte_dirty);
< #else
<       return pte_same(pte, swp_pte);
< #endif
---
>       return pte_same(pte_swp_clear_soft_dirty(pte), swp_pte);
1148c1136,1137
<       if (mem_cgroup_try_charge(page, vma->vm_mm, GFP_KERNEL, &memcg)) {
---
>       if (mem_cgroup_try_charge(page, vma->vm_mm, GFP_KERNEL,
>                               &memcg, false)) {
1154,1155c1143,1144
<       if (unlikely(!maybe_same_pte(*pte, swp_entry_to_pte(entry)))) {
<               mem_cgroup_cancel_charge(page, memcg);
---
>       if (unlikely(!pte_same_as_swp(*pte, swp_entry_to_pte(entry)))) {
>               mem_cgroup_cancel_charge(page, memcg, false);
1166,1167c1155,1156
<               page_add_anon_rmap(page, vma, addr);
<               mem_cgroup_commit_charge(page, memcg, true);
---
>               page_add_anon_rmap(page, vma, addr, false);
>               mem_cgroup_commit_charge(page, memcg, true, false);
1169,1170c1158,1159
<               page_add_new_anon_rmap(page, vma, addr);
<               mem_cgroup_commit_charge(page, memcg, false);
---
>               page_add_new_anon_rmap(page, vma, addr, false);
>               mem_cgroup_commit_charge(page, memcg, false, false);
1212c1201
<               if (unlikely(maybe_same_pte(*pte, swp_pte))) {
---
>               if (unlikely(pte_same_as_swp(*pte, swp_pte))) {
1636,1637d1624
<               struct list_head *lh;
<
1642,1643c1629
<               lh = se->list.next;
<               se = list_entry(lh, struct swap_extent, list);
---
>               se = list_next_entry(se, list);
1667c1653
<               se = list_entry(sis->first_swap_extent.list.next,
---
>               se = list_first_entry(&sis->first_swap_extent.list,
1973c1959
<               mutex_lock(&inode->i_mutex);
---
>               inode_lock(inode);
1975c1961
<               mutex_unlock(&inode->i_mutex);
---
>               inode_unlock(inode);
2200c2186
<               mutex_lock(&inode->i_mutex);
---
>               inode_lock(inode);
2228,2229d2213
<               if (swap_header->info.nr_badpages > MAX_SWAP_BADPAGES)
<                       return 0;
2435c2419
<       /* If S_ISREG(inode->i_mode) will do mutex_lock(&inode->i_mutex); */
---
>       /* If S_ISREG(inode->i_mode) will do inode_lock(inode); */
2580c2564
<                       mutex_unlock(&inode->i_mutex);
---
>                       inode_unlock(inode);
2593c2577
<               mutex_unlock(&inode->i_mutex);
---
>               inode_unlock(inode);
2964,2968c2948,2951
<                       struct list_head *this, *next;
<                       list_for_each_safe(this, next, &head->lru) {
<                               struct page *page;
<                               page = list_entry(this, struct page, lru);
<                               list_del(this);
---
>                       struct page *page, *next;
>
>                       list_for_each_entry_safe(page, next, &head->lru, lru) {
>                               list_del(&page->lru);
