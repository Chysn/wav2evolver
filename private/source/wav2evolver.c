#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "dsi_packing.h"  /* For this, see http://www.dsi-lifeboat.com/viewtopic.php?f=13&t=17 */
#include "pcm_proc.h" /* For this, see http://www.dsi-lifeboat.com/viewtopic.php?f=13&t=130 */

#define PCM_MAX 176400
#define EVOLVER_ID 0x20 /* All Evolvers */
#define EVOLVER_SAMPLE_SIZE 128

int main(int argc, char *argv[])
{
    /* The waveshape number is expected as a command-line argument */
    if (argc < 2) {
        printf("\nusage: %s waveshape_number\n\n", argv[0]);
        return -1;
    }
    int waveshape_number = atoi(argv[1]);
    if (waveshape_number > 128 || waveshape_number < 97) {
        printf("\nwaveshape number out of range (97-128)\n\n");
        return -1;
    }
    waveshape_number--; /* Because waveshape numbers are zero-indexed to the Evolver */

	/* Get the WAV file from standard input */
    signed int wav[PCM_MAX]; /* Entire WAV file */
    unsigned long int size = 0; /* WAV file size */
    for (;;)
    {
    	int b = getchar();
        if (b == EOF || size == PCM_MAX) break;
       	wav[size++] = b;
    }
    
    /* Use pcm_proc tools to convert the WAV file into 16-bit, mono, 128-word PCM */
    PCMData pcm = wav_to_pcm(size, wav);
    pcm_change_resolution(&pcm, 16);
    pcm_change_size(&pcm, EVOLVER_SAMPLE_SIZE);
    pcm = pcm_from_channel(&pcm, PCM_PROC_CHANNEL_LEFT);
    
    /* PCM data is signed, while the dsi_packing tools require data to be unsigned. So,
     * conversion must be done. First, I cast each sample to an int16_t to guarantee that
     * it's a 16-bit signed integer. Then, I divide the 16-bit word into bytes and
     * place them, little-endian, into an evolver data array.
     */
    unsigned int evolver_data[PCM_MAX];
    unsigned long int i; /* i is the index within the PCM data */
    unsigned long int dx = 0; /* dx is the index within the evolver data */
    for (i = 0; i < pcm.size; i++)
    {
    	int16_t sample = (int16_t)pcm.data[i];

        /* Convert the signed 16-bit sample into a high and low byte */
        evolver_data[dx++] = (sample) & 0xff; /* Low byte */
        evolver_data[dx++] = (sample) >> 8; /* High byte */
    }
    
    /* Use dsi_packing tools to convert the sample data into DSI's packed format */
    UnpackedVoice evolver_waveform;
    set_voice_data(&evolver_waveform, dx, evolver_data);
    PackedVoice evolver_sysex = pack_voice(evolver_waveform);
  
    /* Send the actual SysEx to standard output */
    putchar(0xf0); /* Start SysEx */
    putchar(0x01); /* DSI */
    putchar(EVOLVER_ID);
    putchar(0x01); /* File version */
    putchar(0x0a); /* Waveshape Data */
    putchar(waveshape_number);
    dump_voice(evolver_sysex);
    putchar(0xf7); /* End SysEx */

    return 0;
}
