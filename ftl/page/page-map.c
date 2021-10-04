/**
 * @file page-map.c
 * @brief manage the mapping information
 * @author Gijun Oh
 * @version 1.0
 * @date 2021-10-05
 */
#include "include/bits.h"
#include "include/page.h"
#include "include/device.h"
#include "include/log.h"
#include "include/atomic.h"

struct device_address page_ftl_get_page(struct page_ftl *pgftl)
{
	struct device_address paddr;
	struct device *dev;

	struct page_ftl_segment *segment;

	size_t nr_segments;
	size_t pages_per_segment;
	size_t segnum;
	size_t idx;

	uint64_t nr_free_pages;
	uint32_t offset;

	dev = pgftl->dev;
	nr_segments = device_get_nr_segments(dev);
	pages_per_segment = device_get_pages_per_segment(dev);

	paddr.lpn = PADDR_EMPTY;
	idx = 0;

retry:
	if (idx == nr_segments) {
		pr_err("cannot find the free page in the device\n");
		paddr.lpn = PADDR_EMPTY;
		return paddr;
	}
	segnum = (pgftl->alloc_segnum + idx) % nr_segments;
	idx += 1;

	segment = &pgftl->segments[segnum];
	nr_free_pages = atomic_load(&segment->nr_free_pages);
	if (nr_free_pages == 0) {
		goto retry;
	}
	pgftl->alloc_segnum = segnum;

	offset = find_first_zero_bit(segment->used_bits, pages_per_segment, 0);
	paddr.format.block = segnum;
	paddr.lpn |= offset;

	set_bit(segment->used_bits, offset);
	atomic_store(&segment->nr_free_pages, nr_free_pages - 1);

	return paddr;
}

int page_ftl_reclaim_page(struct page_ftl *pgftl, struct device_address paddr)
{
	struct page_ftl_segment *segment;
	struct device *dev;

	size_t pages_per_segment;

	uint64_t nr_free_pages;
	uint32_t offset;

	dev = pgftl->dev;
	pages_per_segment = device_get_pages_per_segment(dev);

	segment = &pgftl->segments[paddr.format.block];

	offset = paddr.lpn % pages_per_segment;
	reset_bit(segment->used_bits, offset);

	nr_free_pages = atomic_load(&segment->nr_free_pages);
	atomic_store(&segment->nr_free_pages, nr_free_pages + 1);
	return 0;
}
