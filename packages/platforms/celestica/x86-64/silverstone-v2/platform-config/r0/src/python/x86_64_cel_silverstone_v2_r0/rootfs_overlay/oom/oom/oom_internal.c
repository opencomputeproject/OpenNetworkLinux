/*
 * Internal OOM functions
 */

#include <stdio.h>
#include <stdint.h>

void print_block_hex(uint8_t* buf)
{
	int j, k;
	uint8_t* bufptr8;
	uint32_t tempintchar;

	bufptr8 = buf;
	for (j = 0; j < 8; j++) {
		printf("       " );
		for (k = 0; k < 19; k++) {
			if ((k % 5) == 4) {
				printf(" ");
			} else {
				tempintchar = *bufptr8;
				printf("%.2X", tempintchar);
				bufptr8++;
			}
		}
		printf("\n");
	}
}
