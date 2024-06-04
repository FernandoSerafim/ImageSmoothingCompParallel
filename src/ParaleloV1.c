#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <sys/time.h>

#define image(x, y) pixels[y * width + x]
#define smooth(x, y) filtered[y * width + x]

typedef struct
{
    char red, green, blue, alpha;
} RGBA;

int main(int argc, char *argv[])
{
    struct timeval start, stop;
    FILE *in, *out;
    short width, height;
    RGBA *pixels, *filtered;
    int DY[] = {-2, -2, -2, -2, -2, -1, -1, -1, -1, -1, +0, +0, +0, +0, +0, +1, +1, +1, +1, +1, +2, +2, +2, +2, +2};
    int DX[] = {-2, -1, +0, +1, +2, -2, -1, +0, +1, +2, -2, -1, +0, +1, +2, -2, -1, +0, +1, +2, -2, -1, +0, +1, +2};
    int x, y, d, dx, dy, i;
    int _;

    in = fopen("image.in", "rb");
    if (in == NULL)
    {
        perror("image.in");
        exit(EXIT_FAILURE);
    }

    out = fopen("imageParallelRobusto.out", "wb");
    if (out == NULL)
    {
        perror("imageParallelRobusto.out");
        exit(EXIT_FAILURE);
    }

    _=fread(&width, sizeof(width), 1, in);
    _=fread(&height, sizeof(height), 1, in);
    _=fwrite(&width, sizeof(width), 1, out);
    _=fwrite(&height, sizeof(height), 1, out);

    pixels = (RGBA *)malloc(height * width * sizeof(RGBA));
    filtered = (RGBA *)malloc(height * width * sizeof(RGBA));
    if (pixels == NULL || filtered == NULL)
    {
        perror("malloc");
        exit(EXIT_FAILURE);
    }   

    gettimeofday(&start, NULL);

    do
    {
        if (!fread(pixels, height * width * sizeof(RGBA), 1, in))
            break;

        #pragma omp parallel 
        for (y = 0; y < height; y++)
        {
            for (x = 0; x < width; x++)
            {

                #pragma omp simd
                for (i = 0; i < 4; i++) // Vetorização explícita
                {
                    long long int sum = 0;

                    //Seria possível paralelizar esse for com parallel for reduction?
                    //Como seria possível melhorar a questao da divisão de trabalho?
                    #pragma omp parallel for reduction(+:sum) 
                    for (d = 0; d < 25; d++)
                    {
                        dx = x + DX[d];
                        dy = y + DY[d];
                        if (dx >= 0 && dx < width && dy >= 0 && dy < height)
                        {
                            sum += *(((char *)(&image(dx, dy))) + i);
                        }
                    }
                    (*(((char *)(&smooth(x, y)) + i))) = sum / 25;
                }
            }
        }

        _=fwrite(filtered, height * width * sizeof(RGBA), 1, out);
    } while (!feof(in));

     gettimeofday(&stop, NULL);

     double t = (((double)(stop.tv_sec)*1000.0  + (double)(stop.tv_usec / 1000.0)) - \
                   ((double)(start.tv_sec)*1000.0 + (double)(start.tv_usec / 1000.0)));

    fprintf(stdout, "Tempo decorrido = %g ms\n", t);

    free(pixels);
    free(filtered);
    fclose(in);
    fclose(out);

    return EXIT_SUCCESS;
}