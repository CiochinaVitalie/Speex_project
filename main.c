#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <speex.h>
#include "lowcfe.h"

#define FRAME_SIZE 160
#define PACKAGE_SIZE 20

long GetFileSize(const char *filename)
{
    long size;
    FILE *f;

    f = fopen(filename, "rb");
    if (f == NULL)
        return -1;
    fseek(f, 0, SEEK_END);
    size = ftell(f);
    fclose(f);

    return size;
}

void data_encode()
{
    printf("SPEEX: Start encode\n");

    int16_t in[FRAME_SIZE];
    char buf[PACKAGE_SIZE];

    // // Create a new encoder state in narrowband mode
    void *encoder_state = speex_encoder_init(&speex_nb_mode);

    // // Set quality to 8 (15 kbps)
    uint32_t tmp = 4;
    speex_encoder_ctl(encoder_state, SPEEX_SET_QUALITY, &tmp);

    FILE *fin = fopen("../sdcard/female.wav", "r");

    long res = GetFileSize("../sdcard/female.wav");
    printf("Size of decoded file is %ld bytes\n", res);

    if (!fin)
    {
        printf("Could not open /sdcard/female.wav\n");
        while (1)
        {
            exit(1);
        }
    }
    FILE *fout = fopen("../sdcard/speex.spx", "w");

    // Initialization of the structure that holds the bits
    SpeexBits bits;
    speex_bits_init(&bits);

    while (1)
    {

        fread(in, 2, FRAME_SIZE, fin);
        if (feof(fin))
            break;
        // Flush all the bits in the struct so we can encode a new frame
        speex_bits_reset(&bits);
        // Encode frame
        speex_encode_int(encoder_state, in, &bits);
        // Copy the bits to an array of char that can be written
        size_t s = speex_bits_write(&bits, buf, PACKAGE_SIZE);
        // write to file
        fwrite(buf, 1, s, fout);
    }
    // Destroy the decoder state
    speex_encoder_destroy(encoder_state);
    // Destroy the bit-stream struct
    speex_bits_destroy(&bits);

    fclose(fout);
    fclose(fin);

    printf("SPEEX: Done encode\n");
}

void data_decode(long percent)
{
    printf("SPEEX: Start decode\n");

    FILE *fin;
    FILE *fout;

    // Holds the audio that will be written to file (16 bits per sample)
    int16_t out[FRAME_SIZE];
    char buf[PACKAGE_SIZE];
    uint32_t lost_prsent = percent;

    static LowcFE_c lc = {0};                  /* PLC simulation data */
    g711plc_construct(&lc);
    // Create a new decoder state in narrowband mode
    void *decoder_state = speex_decoder_init(&speex_nb_mode);

    // Set the perceptual enhancement on
    uint32_t tmp = 1;
    speex_decoder_ctl(decoder_state, SPEEX_SET_ENH, &tmp);

    fin = fopen("../sdcard/speex.spx", "r");

    long res = GetFileSize("../sdcard/speex.spx");
    printf("Size of encoded file is %ld bytes\n", res);

    if (!fin)
    {
        printf("Could not open /sdcard/speex.spx\n");
        while (1)
        {
            exit(1);
        }
    }
    fout = fopen("../sdcard/rawpcm_dec.wav", "w");

    // Initialization of the structure that holds the bits
    SpeexBits bits;
    speex_bits_init(&bits);

    uint16_t los_num_pack = 0;
    uint16_t push_lost = 0;
    uint16_t num_frames = res / PACKAGE_SIZE;
    uint32_t inc = 0;

    if (lost_prsent != 0)
    {
        los_num_pack = (num_frames * lost_prsent) / 100;
        push_lost = num_frames / los_num_pack;
    }

    printf("Number of encoded frames are %d\n", num_frames);
    printf("Number of lost encoded frames are %d\n", los_num_pack);
    

    while (1)
    {
        inc++;

        // Read data from file
        size_t pkg_size = fread(buf, 1, PACKAGE_SIZE, fin);

        if (inc == push_lost)
        {
            inc = 0;
            g711plc_dofe(&lc, out);
            fwrite(out, 2, FRAME_SIZE, fout);
        }

        else
        {
            // Copy the data into the bit-stream struct
            speex_bits_read_from(&bits, buf, pkg_size);
            // Decode the data
            speex_decode_int(decoder_state, &bits, out);
            g711plc_addtohistory(&lc, (short *) out);
            // write to file
            fwrite(out, 2, FRAME_SIZE, fout);
        }

        if (feof(fin))
            break;
    }
    // Destroy the decoder state
    speex_decoder_destroy(decoder_state);
    // Destroy the bit-stream struct
    speex_bits_destroy(&bits);

    fclose(fout);
    fclose(fin);

    printf("SPEEX: Done decode\n");
}

int main(int argc, char **argv)
{
    long arg = strtol(argv[1], NULL, 10);
    // printf("")
    printf("Input lost packets are %ld persents\n", arg);
    data_encode();
    data_decode(arg);
    return 0;
}