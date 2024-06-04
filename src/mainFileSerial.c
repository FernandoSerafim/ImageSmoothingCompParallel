#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <omp.h>

#define image(x,y) pixels[y*width+x]
#define smooth(x,y) filtered[y*width+x]

typedef struct {
    char red, green, blue, alpha;
} RGBA;

int main(int argc, char *argv[]) {
    struct timeval start, stop;
    FILE *in;
    FILE *out;
    int _;

    in = fopen("image.in", "rb");
    if (in == NULL) {
        perror("image.in");
        exit(EXIT_FAILURE);
    }

    out = fopen("imageSeq.out", "wb");
    if (out == NULL) {
        perror("imageSeq.out");
        exit(EXIT_FAILURE);
    }

    short width, height;

    _=fread(&width, sizeof (width), 1, in);
    _=fread(&height, sizeof (height), 1, in);

    _=fwrite(&width, sizeof (width), 1, out);
    _=fwrite(&height, sizeof (height), 1, out);

    RGBA *pixels = (RGBA *) malloc(height * width * sizeof (RGBA));
    RGBA *filtered = (RGBA *) malloc(height * width * sizeof (RGBA));

    if (pixels == NULL || filtered == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    int DY[] = {-2, -2, -2, -2, -2, -1, -1, -1, -1, -1, +0, +0, +0, +0, +0, +1, +1, +1, +1, +1, +2, +2, +2, +2, +2};
    int DX[] = {-2, -1, +0, +1, +2, -2, -1, +0, +1, +2, -2, -1, +0, +1, +2, -2, -1, +0, +1, +2, -2, -1, +0, +1, +2};
    int x, y, d, dx, dy, i;

    gettimeofday(&start, NULL);

    do {
        if (!fread(pixels, height * width * sizeof (RGBA), 1, in))
            break;

        for (y = 0; y < height; y++) {
            for (x = 0; x < width; x++) {
                for (i = 0; i < 4; i++) {
                    long long int sum = 0;
                    for (d = 0; d < 25; d++) {
                        dx = x + DX[d];
                        dy = y + DY[d];
                        if (dx >= 0 && dx < width && dy >= 0 && dy < height) {
                            sum += *(((char*) (&image(dx, dy))) + i);
                        }
                    }
                    (*(((char*) (&smooth(x, y)) + i))) = sum / 25;
                }
            }
        }

        _ = fwrite(filtered, height * width * sizeof (RGBA), 1, out);

    } while (!feof(in));

    gettimeofday(&stop, NULL);

     double t = (((double)(stop.tv_sec)*1000.0  + (double)(stop.tv_usec / 1000.0)) - \
                   ((double)(start.tv_sec)*1000.0 + (double)(start.tv_usec / 1000.0)));

    fprintf(stdout, "Tempo decorrido = %g ms\n", t);

    free(pixels);
    free(filtered);

    fclose(out);
    fclose(in);

    return EXIT_SUCCESS;
}
