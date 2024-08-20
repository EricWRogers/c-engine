#include <vector>
#include <stdio.h>
#include <string.h>
#include <iostream>

#include <SDL3/SDL_cpuinfo.h>
#include <SDL3/SDL_thread.h>
#include <SDL3/SDL_mutex.h>

#include "core/vec.hpp"

#include "core/Types.hpp"
#include "core/String.hpp"

// Two-dimensional value noise based on Hugo Elias's description:
// http://freespace.virgin.net/hugo.elias/models/m_perlin.htm

#include <cstdio>
#include <cmath>
#include <cstdlib>

const int numOctaves = 7;
const double persistence = 0.5;

#define maxPrimeIndex 10

int primes[maxPrimeIndex][3] = {
  { 995615039, 600173719, 701464987 },
  { 831731269, 162318869, 136250887 },
  { 174329291, 946737083, 245679977 },
  { 362489573, 795918041, 350777237 },
  { 457025711, 880830799, 909678923 },
  { 787070341, 177340217, 593320781 },
  { 405493717, 291031019, 391950901 },
  { 458904767, 676625681, 424452397 },
  { 531736441, 939683957, 810651871 },
  { 997169939, 842027887, 423882827 }
};

double Noise(int i, int x, int y) {
  int n = x + y * 57;
  n = (n << 13) ^ n;
  int a = primes[i][0], b = primes[i][1], c = primes[i][2];
  int t = (n * (n * n * a + b) + c) & 0x7fffffff;
  return 1.0 - (double)(t)/1073741824.0;
}

double SmoothedNoise(int i, int x, int y) {
  double corners = (Noise(i, x-1, y-1) + Noise(i, x+1, y-1) +
                    Noise(i, x-1, y+1) + Noise(i, x+1, y+1)) / 16,
         sides = (Noise(i, x-1, y) + Noise(i, x+1, y) + Noise(i, x, y-1) +
                  Noise(i, x, y+1)) / 8,
         center = Noise(i, x, y) / 4;
  return corners + sides + center;
}

double Interpolate(double a, double b, double x) {  // cosine interpolation
  double ft = x * 3.1415927,
         f = (1 - cos(ft)) * 0.5;
  return  a*(1-f) + b*f;
}

double InterpolatedNoise(int i, double x, double y) {
  int integer_X = x;
  double fractional_X = x - integer_X;
  int integer_Y = y;
  double fractional_Y = y - integer_Y;

  double v1 = SmoothedNoise(i, integer_X, integer_Y),
         v2 = SmoothedNoise(i, integer_X + 1, integer_Y),
         v3 = SmoothedNoise(i, integer_X, integer_Y + 1),
         v4 = SmoothedNoise(i, integer_X + 1, integer_Y + 1),
         i1 = Interpolate(v1, v2, fractional_X),
         i2 = Interpolate(v3, v4, fractional_X);
  return Interpolate(i1, i2, fractional_Y);
}

double ValueNoise_2D(double x, double y) {
  double total = 0,
         frequency = pow(2, numOctaves),
         amplitude = 1;
  for (int i = 0; i < numOctaves; ++i) {
    frequency /= 2;
    amplitude *= persistence;
    total += InterpolatedNoise(i % maxPrimeIndex,
        x / frequency, y / frequency) * amplitude;
  }
  return total / frequency;
}

int* myVector; 
// Mutex for safe access to the vector 
SDL_Mutex *mutex;

enum { width = 550, height = 400 };

struct RandomData
{
    u32 seed;
};

void SaveImageToFile(const char* filename, std::vector<u8> _image, u32 _width, u32 _height)
{   
    const int MaxColorComponentValue = 255;
    FILE * fp;
    /* comment should start with # */
    const char *comment = "# this is my new binary pgm file";

    /* write the whole data array to ppm file in one step */
    /* create new file, give it a name and open it in binary mode */
    fp = fopen(filename, "wb");

    /* write header to the file */
    fprintf(fp, "P5\n %s\n %d\n %d\n %d\n",
    comment, _width, _height, MaxColorComponentValue);

    /* write image data bytes to the file */
    fwrite(_image.data(), _image.size() * sizeof(unsigned char), 1, fp);
    fclose(fp);
    printf("OK - file %s saved\n", filename);
}

// Function to add an element to the vector safely 
int GeneratePerlinNoise(void *_data) 
{ 
    RandomData* data = (RandomData*)_data;
    u32 height = 256;
    u32 width = 256;
    std::vector<unsigned char> image = {};
    u32 tempSeed = data->seed;

    double largestValue = 0.0;

    srand(tempSeed);

    for (int y = 0; y < height; ++y)
    {
        for (int x = 0; x < width; ++x)
        {
            //image.push_back(rand_r(&tempSeed) % u8_max);
            image.push_back(ValueNoise_2D(x + (width * data->seed), y) * 2.0 * u8_max);

            if (ValueNoise_2D(x + (width * data->seed), y) * 2.0 > largestValue)
            {
                largestValue = ValueNoise_2D(x + (width * data->seed), y) * 2.0;
            }
        }
    }

    String fileName = "seed_" + ToString(data->seed);
    fileName += ".pgm";
    SaveImageToFile( fileName.CString(), image, width, height);
    
    SDL_LockMutex(mutex);
	  myVector[data->seed] = data->seed;
    printf("%lf", largestValue);
    SDL_UnlockMutex(mutex);
    return 0;
} 

int main() 
{
    const u32 CPUCOUNT = SDL_GetCPUCount();
    SDL_Thread* threads[CPUCOUNT];
    RandomData randomData[CPUCOUNT];
    int dumbSharedArray[CPUCOUNT];

    myVector = dumbSharedArray;

    mutex = SDL_CreateMutex();

    for (int i = 0; i < CPUCOUNT; i++)
    {
        randomData[i].seed = i;
        threads[i] = SDL_CreateThread(GeneratePerlinNoise, "thread_test", (void*)&randomData[i]);
    }

    int threadReturnValue = 0;
    for (int i = 0; i < CPUCOUNT; i++)
    {
        SDL_WaitThread(threads[i], &threadReturnValue);
    }

	// Display the elements of the vector 
	printf("Vector elements: ");

	for (int i = 0; i < CPUCOUNT; i++) {
    printf("%d ", myVector[i]);
	}
  
  printf("\n");

    SDL_DestroyMutex(mutex);

	return 0; 
}
