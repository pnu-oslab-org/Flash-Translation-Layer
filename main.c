/**
 * @file main.c
 * @brief main program for test the interface
 * @author Gijun Oh
 * @version 1.0
 * @date 2021-09-22
 */
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "include/module.h"
#include "include/flash.h"
#include "include/log.h"
#include "include/device.h"

int main(void)
{
	struct flash_device *flash = NULL;
	char buffer[8192];
	assert(0 == module_init(PAGE_FTL_MODULE, &flash, RAMDISK_MODULE));
	pr_info("module initialize\n");
	flash->f_op->open(flash);
	for (int i = 0; i < 8192 * 3; i++) {
		int num;
		num = i * 2;
		memset(buffer, 0, 8192);
		*(int *)buffer = num;
		flash->f_op->write(flash, buffer, 512, i * 512);
		num = i * 2 + 1;
		memset(buffer, 0, 8192);
		*(int *)buffer = num;
		pr_info("write value: %d\n", *(int *)buffer);
		flash->f_op->write(flash, buffer, 512, i * 512);
		flash->f_op->read(flash, buffer, 512, i * 512);
		memset(buffer, 0, 8192);
		pr_info("read value: %d\n", *(int *)buffer);
	}
	for (int i = 0; i < 8192 * 3; i++) {
		flash->f_op->read(flash, buffer, 512, i * 512);
		pr_info("read value: %d\n", *(int *)buffer);
	}
	flash->f_op->close(flash);
	assert(0 == module_exit(flash));
	pr_info("module deallcation\n");

	return 0;
}
