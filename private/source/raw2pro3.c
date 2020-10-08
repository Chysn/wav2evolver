#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "dsi_packing.h"
#include "pcm_proc.h"

#define PCM_MAX 176400
#define PRO3_SAMPLE_SIZE 1024

int main(int argc, char *argv[])
{
    /* The wavetable number is expected as a command-line argument */
    if (argc < 3) {
        printf("\nusage: %s wavetable_name wavetable_number\n\n", argv[0]);
        return -1;
    }
    int wavetable_number = atoi(argv[2]);
    if (wavetable_number > 64 || wavetable_number < 33) {
        printf("\nwavetable number out of range (33-64)\n\n");
        return -1;
    }
    wavetable_number--; /* Because wavetable numbers are zero-indexed to the Pro3 */

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
    
    /* PCM data is signed, while the dsi_packing tools require data to be unsigned. So,
     * conversion must be done. First, I cast each sample to an int16_t to guarantee that
     * it's a 16-bit signed integer. Then, I divide the 16-bit word into bytes and
     * place them, big-endian, into an pro3 data array.
     */
    unsigned int pro3_data[PCM_MAX];
    unsigned long int i; /* i is the index within the PCM data */
    unsigned long int dx = 0; /* dx is the index within the evolver data */    
    
	int k;
    for (k = 0; k < 16; k++)
    {
	    /* Use pcm_proc tools to convert the RAW pcm into 16-bit, mono, 128-word PCM */
	    PCMData pcm = new_PCMData();
	    set_pcm_data(&pcm, 128, raw);
	    pcm_change_resolution(&pcm, 16);
	    pcm_change_size(&pcm, PRO3_SAMPLE_SIZE);
	    pcm = pcm_from_channel(&pcm, PCM_PROC_CHANNEL_LEFT);
	    
	    int i;
	    for (i = 0; i < pcm.size; i++)
	    {
	    	int16_t sample = (int16_t)pcm.data[i];
	
	        /* Convert the signed 16-bit sample into a high and low byte */
	        pro3_data[dx++] = (sample) >> 8; /* High byte */
	        pro3_data[dx++] = (sample) & 0xff; /* Low byte */
	    }	    
	    
	    /* Downsample to a 512-word waveform */
	    pcm_change_size(&pcm, PRO3_SAMPLE_SIZE / 2);
	    for (i = 0; i < pcm.size; i++)
	    {
	    	int16_t sample = (int16_t)pcm.data[i];
	        pro3_data[dx++] = (sample) >> 8; /* High byte */
	        pro3_data[dx++] = (sample) & 0xff; /* Low byte */
	    }
	    
	    /* Downsample to a 256-word waveform and add it twice */
	    pcm_change_size(&pcm, PRO3_SAMPLE_SIZE / 4);
	    int j;
	    for (j = 0; j < 2; j++) 
	    {
		    for (i = 0; i < pcm.size; i++)
		    {
		    	int16_t sample = (int16_t)pcm.data[i];
		        pro3_data[dx++] = (sample) >> 8; /* High byte */
		        pro3_data[dx++] = (sample) & 0xff; /* Low byte */
		    }
		}
	    
	    /* Downsample to a 128-word waveform and add it eight times */
	    pcm_change_size(&pcm, PRO3_SAMPLE_SIZE / 8);
	    for (j = 0; j < 8; j++) 
	    {
		    for (i = 0; i < pcm.size; i++)
		    {
		    	int16_t sample = (int16_t)pcm.data[i];
		        pro3_data[dx++] = (sample) >> 8; /* High byte */
		        pro3_data[dx++] = (sample) & 0xff; /* Low byte */
		    }
		}		
	}
	
	pro3_data[dx++] = pro3_data[0];
	pro3_data[dx++] = pro3_data[1];
	    
    /* Use dsi_packing tools to convert the sample data into DSI's packed format */
    UnpackedVoice pro3_wavetable;
    set_voice_data(&pro3_wavetable, dx, pro3_data);
    PackedVoice pro3_sysex = pack_voice(pro3_wavetable);
  
    /* Send the actual SysEx to standard output */
    putchar(0xf0); /* Start SysEx */
    putchar(0x01); /* DSI */
    putchar(0x31); /* Pro3 */
    putchar(0x6a);
    putchar(0x6c);
    putchar(0x01);
    putchar(0x6b);
    putchar(wavetable_number);
    for (i = 0; i < 8; i++) putchar(argv[1][i]); /* Wavetable name */
    putchar(0x00);
    dump_voice(pro3_sysex);
    putchar(0xf7); /* End SysEx */

    return 0;
}
