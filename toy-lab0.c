#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define CACHE_LINE_SIZE 64
#define STR_LENGTH	(CACHE_LINE_SIZE << 2)

void mfence() {
        asm volatile("sfence":::"memory");
}

void clflush(char *data, int len) {
	volatile char *ptr = (char *)((unsigned long)data &~(CACHE_LINE_SIZE-1));
	mfence();
	for(; ptr < (data + len); ptr += CACHE_LINE_SIZE) {
		fprintf(stdout, "<%s, %s, %d> clflush to be run\n", __FILE__, __func__, __LINE__);
		asm volatile("clflush %0" : "+m" (*(volatile char *)ptr));
	}
	mfence();
}

int main(int argc, char *argv[]) {
	char *src = NULL, *dst = NULL;
	int i = 0;
	fprintf(stdout, "User program begins\n");
	assert(posix_memalign((void **)&src, CACHE_LINE_SIZE, STR_LENGTH) == 0);
	assert(posix_memalign((void **)&dst, CACHE_LINE_SIZE, STR_LENGTH) == 0);
	for (; i < STR_LENGTH; ++i)
		src[i] = 0x41 + i % 26;

	memcpy(dst, src, STR_LENGTH);
	clflush(dst, STR_LENGTH);
	free(src);
	free(dst);
	fprintf(stdout, "User program ends\n");
	return 0;
}
