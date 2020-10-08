#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pcm_proc.h"

#define PCM_MAX 176400

int main(int argc, char *argv[])
{
    /* The wavetable number is expected as a command-line argument */
    if (argc < 2) {
        printf("\nusage: %s size\n\n", argv[0]);
        return -1;
    }
   	int new_size = atoi(argv[1]);

	/* Get the RAW file from standard input */
    signed int raw[PCM_MAX]; /* Entire RAW file */
    unsigned long int size = 0; /* RAW file size */
    for (;;)
    {
    	int lb = getchar(); /* Low byte */
    	int hb = getchar(); /* High byte */
        if (hb == EOF || size == PCM_MAX) break;
       	raw[size++] = (hb << 8) + lb;
    }
    
    PCMData pcm = new_PCMData();
    set_pcm_data(&pcm, size, raw);
    pcm_change_resolution(&pcm, 16);
    pcm_change_size(&pcm, new_size);
    pcm = pcm_from_channel(&pcm, PCM_PROC_CHANNEL_LEFT);
    
	int i;
    for (i = 0; i < pcm.size; i++) 
    {
    	int sample = pcm.data[i];
        putchar((sample) >> 8);   /* High byte */
        putchar((sample) & 0xff); /* Low byte */
    }

    return 0;
}
