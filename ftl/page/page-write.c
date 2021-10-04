/**
 * @file page-write.c
 * @brief write logic for page ftl
 * @author Gijun Oh
 * @version 1.0
 * @date 2021-09-22
 */
#include "include/module.h"
#include "include/page.h"
#include "include/device.h"
#include "include/log.h"

#include <pthread.h>
#include <assert.h>
#include <stdlib.h>

static struct device_request *
page_ftl_merge_request(struct device_request *rq_vec)
{
	struct device_request *request;
	struct page_ftl *pgftl;
	struct device *dev;
	size_t page_size;
	char *buffer;

	request = NULL;
	buffer = NULL;

	pgftl = (struct page_ftl *)rq_vec->rq_private;
	dev = pgftl->dev;
	page_size = device_get_page_size(dev);
	request =
		(struct device_request *)malloc(sizeof(struct device_request));
	if (request == NULL) {
		pr_err("memory allocation failed\n");
		goto exception;
	}

	buffer = (char *)malloc(page_size);
	if (buffer == NULL) {
		pr_err("memory allocation failed\n");
		goto exception;
	}

	request->paddr = page_ftl_get_page(pgftl);
	request->data_len = page_size;
	request->data = buffer;
	/**< data must merge in here !!! you must implementate!! */
	request->rq_private = rq_vec;
	return request;
exception:
	if (buffer) {
		free(buffer);
	}
	if (request) {
		free(request);
	}
	return NULL;
}

static int page_ftl_vectored_write(struct device_request *rq_vec)
{
	struct device_request *request;
	request = page_ftl_merge_request(rq_vec);
	return 0;
}

int page_ftl_fill_wb(const uint64_t lpn, uintptr_t __request)
{
	struct page_ftl *pgftl;
	struct device_request *request;
	struct device *dev;

	(void)lpn;

	request = (struct device_request *)__request;
	pgftl = (struct page_ftl *)request->rq_private;
	dev = pgftl->dev;

	assert(NULL != request);
	assert(NULL != dev);

	pthread_mutex_lock(&dev->mutex);
	if (dev->inflight_request) {
		struct device_request *rq_vec;
		rq_vec = dev->inflight_request;
		dev->inflight_request = NULL;
		pthread_mutex_unlock(&dev->mutex);

		rq_vec->next_rq = request;
		return page_ftl_vectored_write(rq_vec);
	}
	dev->inflight_request = request;
	pthread_mutex_unlock(&dev->mutex);
	return 0;
}

ssize_t page_ftl_write(struct page_ftl *pgftl, struct device_request *request)
{
	request->next_rq = NULL;
	request->rq_private = (void *)pgftl;
	return 0;
}
