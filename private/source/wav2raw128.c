#include <stdio.h>
#include "pcm_proc.h"

int main()
{
    signed int wav[PCM_PROC_MAX];
    unsigned long int size = 0;
    int c;
    for (;;)
    {
        c = getchar();
        if (c == EOF) {break;}
        wav[size++] = c;
    }
    
    PCMData pcm = wav_to_pcm(size, wav);
    pcm = pcm_from_channel(&pcm, PCM_PROC_CHANNEL_LEFT);
    pcm_change_resolution(&pcm, 16);
    pcm_change_size(&pcm, 128);
    
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
