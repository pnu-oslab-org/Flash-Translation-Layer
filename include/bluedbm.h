#ifndef BLUEDBM_H
#define BLUEDBM_H

#include <stdint.h>

#include "device.h"

/**
 * @brief structure for manage the bluedbm
 */
struct bluedbm {
	size_t size;
};

int bluedbm_open(struct device *, const char *name, int flags);
ssize_t bluedbm_write(struct device *, struct device_request *);
ssize_t bluedbm_read(struct device *, struct device_request *);
int bluedbm_erase(struct device *, struct device_request *);
int bluedbm_close(struct device *);

int bluedbm_device_init(struct device *, uint64_t flags);
int bluedbm_device_exit(struct device *);

#endif
