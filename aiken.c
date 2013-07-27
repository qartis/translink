#include <sndfile.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define MAX 512

char *dab(short int *sample, int sample_size, int silence_thres)
{
	int i = 0, peak = 0, ppeak = 0;
    int peaks[MAX];
    char str[MAX];
    char *ptr = str;
	int peaks_size = 0;
	
	for (i=0; i < sample_size; i++){
        if (sample[i] < 0)
            sample[i] = -sample[i];

        if (sample[i] < 0)
            sample[i] = SHRT_MAX;
    }

    float a = 0.5;
    for(i=1;i<sample_size;i++){
        sample[i] = a * sample[i] + (1.0 - a) * sample[i-1];
    }

	i = 0;
	while (i < sample_size && peaks_size < MAX) {
		ppeak = peak;
		while (sample[i] <= silence_thres && i < sample_size)
			i++;
		peak = 0;
		while (sample[i] > silence_thres && i < sample_size) {
			if (sample[i] > sample[peak])
				peak = i;
			i++;
		}
		if (peak - ppeak > 0) {
			peaks[peaks_size] = peak - ppeak;
			peaks_size++;
		}
	}

    int prev_zero_width = peaks[1];

	for (i = 1; i < peaks_size-1; i++) {
        int dist_to_0 = abs(peaks[i] - prev_zero_width);
        int dist_to_1 = abs(peaks[i] + peaks[i+1] - prev_zero_width);
        if (dist_to_0 < dist_to_1){
            prev_zero_width = peaks[i];
            *ptr++ = '0';
        } else if (dist_to_1 < dist_to_0){
            prev_zero_width = peaks[i] + peaks[i+1];
            *ptr++ = '1';
            i++;
        } else {
            *ptr++ = 'x';
        }
	}

    *ptr = '\0';
    return strdup(str);
}

int decode(char *str);

int main(int argc, char *argv[])
{
    short int *sample = NULL;
    int sample_size = 0;

	SNDFILE *sndfile;
	SF_INFO sfinfo = {0};

    if (argc != 3){
        printf("usage: %s <filename> <silence threshold>\n", argv[0]);
        return 0;
    }

    char *filename = argv[1];
    int silence = atoi(argv[2]);
	
    sndfile = sf_open(filename, SFM_READ, &sfinfo);
    assert(sfinfo.channels == 1);
    sample_size = sfinfo.frames;
    sample = malloc(sizeof(short int) * sample_size);
    sf_read_short(sndfile, sample, sample_size);
	

    int i;
    for(i=0;i<2;i++) {
        for (silence = 500; silence < 20000; silence += 500) {
            char *str = dab(sample, sample_size, silence);
            if (strlen(str) < 10) {
                continue;
            }
            printf("str: '%s'\n", str);

            size_t leading_zeroes = strspn(str, "0");
            if (leading_zeroes != 7) {
                printf("broken header (only %ld leading zeroes), repairing..\n", leading_zeroes);
                char *fixed = malloc(strlen(str) * 2);
                strcpy(fixed, "0000000");
                strcat(fixed, str + leading_zeroes);
                str = fixed;
            }

            if (!decode(str)) {
                printf("decoded: %s\n", str);
                printf("silence: %d\n", silence);
                exit(0);
            }
        }

        int end = sample_size - 1;
        int j;
        for (j = 0; j < sample_size / 2; j++) {
            short int tmp = sample[j];
            sample[j] = sample[end];
            sample[end] = tmp;
            end--;
        }
    }

    return 0;
}

#define main foo
#include "decode.c"
