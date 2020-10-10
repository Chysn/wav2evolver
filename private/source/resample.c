#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main(int argc, char *argv[])
{
    int checksum = 0;

	/* Get the RAW file from standard input */
    for (;;)
    {
    	int b = getchar();
        if (b == EOF) break;
    	checksum += b;
    }
    
    printf("\nSum: %d\n\n", checksum);
    
    return 0;
}
