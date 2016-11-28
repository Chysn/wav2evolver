/* DSI Packing (dsi_packing.h)
 *
 * DSI System Exclusive data is arranged in 8-byte packets.  The first
 * byte in each packet is a composite of the high bits of the next
 * seven data bytes.
 *
 * Four functions are provided in this header.  unpack_voice() converts a
 * single DSI voice's system exclusive data into a series of data bytes,
 * so that the voice data may be freely manipulated.
 *
 * pack_voice() converts an array of DSI voice data into the packed format,
 * so that it may be transmitted via system exclusive to the appropriate
 * synth.
 *
 * set_voice_data() is a shortcut for putting a data array into a DSIVoiceData
 * struct, given its size and the data array.
 *
 * dump_voice() sends the voice data to standard output.
 *
 * Please see the bottom for boring license information.
 */
#ifndef DSI_PACKING_H_
#include <stdio.h>
#define DSI_PACKING_H_
#define DSI_VOICE_DATA_MAX 1024

/*
 * DSIVoiceData is a sort of generic data structure, containing a size, and a fixed-length
 * array of data.  Since it's possible to use the same data structure to represent both packed
 * and unpacked data, several typedefs are included to disambiguate the role of the data in
 * your code.
 */
typedef struct _DSIVoiceData {
	int size;
	unsigned int data[DSI_VOICE_DATA_MAX];
} DSIVoiceData, UnpackedVoice, PackedVoice;

/* Function declarations */
UnpackedVoice unpack_voice(PackedVoice packed);
PackedVoice pack_voice(UnpackedVoice unpacked);
void set_voice_data(DSIVoiceData *voice, int size, unsigned int data[]);
void dump_voice(DSIVoiceData voice);

/*
 * Given packed voice data (for example, the data that would come directly from a DSI instrument's
 * system exclusive dump), unpack_voice() returns an UnpackedVoice, whose data property contains a
 * one-byte-per-parameter representation of the voice data.  The UnpackedVoice will be an easy way
 * to examine, manipulate, and modify voice data.  Example:
 *
 *   (Accept a system exclusive dump.  Process the sysex header yourself, and then put the data into
 *      data, an array of unsigned ints.  Store the size of the data array in int size)
 *   PackedVoice packed_sysex;
 *   set_voice_data(&packed_sysex, size, data);
 *   UnpackededVoice mopho_voice = unpack_voice(packed_sysex);
 *   int cutoff_frequency = mopho_voice.data[20];
 */
UnpackedVoice unpack_voice(PackedVoice packed)
{
    unsigned int data[DSI_VOICE_DATA_MAX];
    int packbyte = 0;  /* Composite of high bits of next 7 bytes */
    int pos = 0;       /* Current position of 7 */
    int ixp;           /* Packed byte index */
    int size = 0;      /* Unpacked voice size */
    unsigned int c;   /* Current source byte */
    for (ixp = 0; ixp < packed.size; ixp++)
    {
        c = packed.data[ixp];
        if (pos == 0) {
            packbyte = c;
        } else {
            if (packbyte & (1 << (pos - 1))) {c |= 0x80;}
            data[size++] = c;
        }
        pos++;
        pos &= 0x07;
        if (size > DSI_VOICE_DATA_MAX) break;
    }
    
    UnpackedVoice unpacked;
    set_voice_data(&unpacked, size, data);
    return unpacked;
}


/*
 * Given unpacked voice data (for example, data that might be modified or created by calling software),
 * pack_voice() returns a PackedVoice, whose data property contains a packed representation of the
 * voice data.  The PackedVoice is suitable for sending back to a DSI instrument.  To send the packed
 * data back via a file or direct I/O call, see dump_voice(); or roll your own I/O to MIDI.  Don't forget
 * the appropriate system exclusive header. Example:
 *
 *   (Let's continue the example from unpack_voice() above, by opening the filter all the way.  As you
 *     may recall, mopho_voice_data is an UnpackedVoice)
 *   mopho_voice.data[20] = 164;
 *   PackedVoice mopho_sysex = pack_voice(mopho_voice);
 */
PackedVoice pack_voice(UnpackedVoice unpacked)
{
    unsigned int data[DSI_VOICE_DATA_MAX];
    int packbyte = 0;  /* Composite of high bits of next 7 bytes */
    int pos = 0;       /* Current position of 7 */
    int ixu;           /* Unpacked byte index */
    int size = 0;      /* Packed voice size */
    int packet[7];     /* Current packet */
    int i;             /* Packet output index */
    unsigned int c;    /* Current source byte */
    for (ixu = 0; ixu < unpacked.size; ixu++)
    {
        c = unpacked.data[ixu];
        if (pos == 7) {
        	data[size++] = packbyte;
            for (i = 0; i < pos; i++)
            {
            	data[size++] = packet[i];
            }
            packbyte = 0;
            pos = 0;
        }
        if (c & 0x80) {
            packbyte += (1 << pos);
            c &= 0x7f;
        }
        packet[pos] = c;
        pos++;
        if ((size + 8) > DSI_VOICE_DATA_MAX) break;
    }
    data[size++] = packbyte;
    for (i = 0; i < pos; i++) data[size++] = packet[i];
    PackedVoice packed;
    set_voice_data(&packed, size, data);
    return packed;
}


/*
 * Update an UnpackedVoice or a PackedVoice with the size and data array.
 *
 * If this was object-oriented code, this would be part of the constructor.  But since this is C,
 * and DSIVoiceData is a struct instead of a class, populating it is a two-step process.  If you
 * have a data array (an array of unsigned ints representing the packed or unpacked data) called
 * data, and know its size, you set up the struct like (for example) this:
 *
 *   UnpackedVoice voice;
 *   set_voice_data(&voice, size, data);
 *
 * Now you can access the individual parameters with voice.data[index].  See dump_voice(), below,
 * for a really simple example of this.
 */
void set_voice_data(DSIVoiceData *voice, int size, unsigned int data[])
{
	voice->size = size;
	int i;
	for (i = 0; i <  size; i++) voice->data[i] = data[i];
}


/*
 * Send the voice data to standard output.  The data may be either packed or unpacked.  If 
 * it's packed, it can be used to create a system exclusive file.  Note that this is only the
 * parameter data and not the system exclusive header.
 */
void dump_voice(DSIVoiceData voice)
{
	int i;
	for (i = 0; i < voice.size; i++) putchar(voice.data[i]);
}

#endif /* DSI_PACKING_H_ */

/*
 * Copyright (c) 2010, 2012, 2013 The Beige Maze Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
