#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/file.h>
#include "uart.h"
#include "spi.h"
#include "terminal.h"
#include "menu.h"
#include "sd.h"
#include "input.h"
#include "hexload.h"
#include "rom.h"
#include "fat.h"
#include "cache.h"
#include "filebrowse.h"
#include "memtest.h"
#include "serial-transfer.h"
#include "main.h"

static void clear_and_print(void *arg);
static void test_sdram(void *arg);
static void autoboot(void *arg);
static void test_dir(void *arg);

uint8_t fat_buf[512];

static const char *sd_card_type_name[] = {
	[SD_CARD_TYPE_MMC] = "MMC",
	[SD_CARD_TYPE_SD] = "SD",
	[SD_CARD_TYPE_SDHC] = "SDHC",
};

static const char *fat_type_name[] = {
	[FAT_TYPE_FAT16] = "FAT16",
	[FAT_TYPE_FAT32] = "FAT32",
};

Menu menu_main = {
	clear_and_print,
	"Trollectronics Trollbook BIOS\nMain menu\n----------------------------------------\n",
	false,
	0,
	7,
	{
		{"Boot kernel.elf", autoboot, (void *) false},
		{"Debug kernel.elf", autoboot, (void *) true},
		{"Browse SD card filesystem", menu_execute, &menu_dir},
		{"Serial file transfer to SD", serial_transfer_recv, NULL},
		{"SDRAM Memtest", test_sdram, NULL},
		{"Test dir", test_dir, NULL},
		{"Reboot", reboot, NULL},
	},
};


static int fat_read_sd(uint32_t sector, uint8_t *data) {
	SDStreamStatus status;
	
	status = SD_STREAM_STATUS_BEGIN;
	
	sd_stream_read_block(&status, sector);
	if(status == SD_STREAM_STATUS_FAILED)
		return -1;
	
	while(status >= 1)
		*data++ = sd_stream_read_block(&status);
	
	if(status == SD_STREAM_STATUS_FAILED)
		return -1;
	
	return 0;
}

static int fat_write_sd(uint32_t sector, uint8_t *data) {
	SDStreamStatus status;
	
	status = SD_STREAM_STATUS_BEGIN;
	
	sd_stream_write_block(&status, sector);
	if(status == SD_STREAM_STATUS_FAILED)
		return -1;
	
	while(status >= 1)
		sd_stream_write_block(&status, *data++);
	
	if(status == SD_STREAM_STATUS_FAILED)
		return -1;
	
	return 0;
}

void print_filesize(uint32_t filesize) {
	if(filesize < 1024)
		printf("%u", filesize);
	else if(filesize < 1024*1024)
		printf("%uk", filesize/1024U);
	else
		printf("%uM", filesize/(1024U*1024U));
}

void sprint_filesize(char *buf, uint32_t filesize) {
	if(filesize < 1024)
		sprintf(buf, "%u", filesize);
	else if(filesize < 1024*1024)
		sprintf(buf, "%uk", filesize/1024U);
	else
		sprintf(buf, "%uM", filesize/(1024U*1024U));
}

static void clear_and_print(void *arg) {
	static bool clear = false;
	
	if(clear)
		terminal_clear();
	
	clear = true;
	printf("%s", arg);
}

static void test_sdram(void *arg) {
	terminal_clear();
	
	printf("Performing memtest without the cache\n");
	memtest_run(false);
	printf("Performing memtest with the cache\n");
	memtest_run(true);
	
	input_poll();
}


static void test_dir(void *arg) {
	DIR *d;
	struct dirent *ent;
	
	terminal_clear();
	
	d = opendir("/");
	
	while((ent = readdir(d))) {
		printf("%s\n", ent->d_name);
	}
	
	closedir(d);
	
	for(;;);
	input_poll();
}

static void autoboot(void *arg) {
	bool debug = (bool) arg;
	execute_elf_path("/KERNEL.ELF", debug);
}

size_t write_terminal(const void *ptr, size_t size, File *f) {
	size_t i;
	char *s = (char *) ptr;
	
	for(i = 0; i < size; i++)
		terminal_putc_term(*s++);
	
	return size;
}

FileHandler fh_terminal = {
	.open = NULL,
	.close = NULL,
	.read = NULL,
	.write = write_terminal,
};

File file_terminal = {
	.handler = &fh_terminal,
};

int main() {
	int type;
	char label[12];
	
	terminal_init();
	
	stdin = (FILE *) &file_terminal;
	stdout = (FILE *) &file_terminal;
	stderr = (FILE *) &file_terminal;
	
	printf("Detecting SD card: ");
	if((type = sd_init()) == SD_CARD_TYPE_INVALID) {
		goto fail;
	}
	
	terminal_set_fg(TERMINAL_COLOR_LIGHT_GREEN);
	printf("%s\n", sd_card_type_name[type]);
	terminal_set_fg(TERMINAL_COLOR_WHITE);
	printf(" - Card size: ");
	print_filesize(sd_get_card_size()/2*1024);
	printf("B\n");
	
	printf("Detecting file system: ");
	if(fat_init(fat_read_sd, fat_write_sd, fat_buf) < 0) {
		goto fail;
	}
	
	type = fat_type();
	terminal_set_fg(TERMINAL_COLOR_LIGHT_GREEN);
	printf("%s\n", fat_type_name[type]);
	terminal_set_fg(TERMINAL_COLOR_WHITE);
	fat_get_label(label);
	printf(" - Volume label: %s\n\n", label);
	
	menu_execute(&menu_main);
	
	for(;;);
	
	fail:
	terminal_set_fg(TERMINAL_COLOR_LIGHT_RED);
	printf("failed");
	terminal_set_fg(TERMINAL_COLOR_WHITE);
	
	for(;;);
		
	return 0;
}


