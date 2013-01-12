#ifndef PANIC_TRANSFER
#define PANIC_TRANSFER
/*
	panic transfer.c

*/

#define CRASH_LOG_SIZE (256*1024)   /* 256k */

int mmc_panic_save_card(void *card);
int mmc_panic_read(u8* buffer, unsigned int length);
int mmc_panic_write(u8* buffer, unsigned int length);

#endif
