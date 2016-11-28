#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pcm_proc.h" /* For this, see http://www.dsi-lifeboat.com/viewtopic.php?f=13&t=130 */

#define PCM_MAX 256

int main()
{
	/* Get the RAW file from standard input */
    signed int raw[PCM_MAX]; /* Entire RAW file */
    unsigned long int size = 0; /* RAW file size */
    for (;;)
    {
    	int lb = getchar(); /* Low byte */
    	int hb = getchar(); /* High byte */
        if (hb == EOF || size == PCM_MAX) break;

        int16_t sample = hb * 256 + lb;
       	raw[size++] = (signed int)sample;
    }

    /* Use pcm_proc tools to convert the RAW pcm into 16-bit, mono, 128-word PCM */
    PCMData pcm = new_PCMData();
    set_pcm_data(&pcm, 128, raw);
    pcm_normalize(&pcm, 1);

    int i;
    for (i = 0; i < pcm.size; i++)
    {
        unsigned int sample = pcm.data[i];

        /* Convert the signed 16-bit sample into a high and low byte */
        char lo = (sample) & 0xff; /* Low byte */
        char hi = (sample) >> 8; /* High byte */

        putchar(lo);
        putchar(hi);
    }
    return 0;
}
