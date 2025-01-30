// MIT License
// 
// Copyright (c) 2023 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <stdint.h>
#include "utility.h"
#include "star.h"
#include "float.h"
#include <sys/time.h>
#include <pthread.h>

#define NUM_STARS 30000 
#define MAX_LINE 1024
#define DELIMITER " \t\n"

struct Star star_array[NUM_STARS] __attribute__((aligned(64)));
uint8_t (*distance_calculated)[NUM_STARS] __attribute__((aligned(64)));

double min = FLT_MAX;
double max = FLT_MIN;

int nthreads = 0;

//timer function
double timer(char* desc) {
  static struct timeval start;
  struct timeval end;
  double ret;

  //start timer
  if (!desc) {
    gettimeofday(&start, NULL);
    return 0.0;
  }

  //ends timer
  gettimeofday(&end, NULL);
  ret = (end.tv_usec - start.tv_usec) * .000001 + (end.tv_sec - start.tv_sec);
  printf("%s: %lf\n", desc, ret);

  return ret;
}

void showHelp() {
  printf("Use: findAngular [options]\n");
  printf("Where options are:\n");
  printf("-t          Number of threads to use\n");
  printf("-h          Show this help\n");
}

float determineAverageAngularDistance(struct Star arr[]) {
  double mean = 0;

  uint32_t i, j;
  uint64_t count = 0;

  for (i = 0; i < NUM_STARS; i++) {
    for (j = 0; j < NUM_STARS; j++) {
      if (i != j && distance_calculated[i][j] == 0) {
        double distance = calculateAngularDistance(arr[i].RightAscension, arr[j].Declination, arr[j].RightAscension, arr[j].Declination);
        distance_calculated[i][j] = 1;
        distance_calculated[j][i] = 1;
        count++;

        if (min > distance) {
          min = distance;
        }

        if (max < distance) {
          max = distance;
        }

        mean = mean + (distance - mean) / count;
      }
    }
  }
  return mean;
}

// struct to store info for each thread
struct tctl{
  int id, count;
  double min, max, total;
  struct Star* arr;
  pthread_t tid;
};

// thread function
void* tfun(void* arg){
  struct tctl* t = arg;
  uint32_t i, j, owner = 0;
  t->min = FLT_MAX;
  t->max = FLT_MIN;

  //divide the work
  for (i = t->id; i < NUM_STARS; i += nthreads) {
    for (j = i + 1; j < NUM_STARS; j++) {
      t->count++;
      double distance = calculateAngularDistance(t->arr[i].RightAscension, t->arr[j].Declination, t->arr[j].RightAscension, t->arr[j].Declination);
      t->total += distance;

      if (t->min > distance){
        t->min = distance;
      }

      if (t->max < distance){
        t->max = distance;
      }
    }
  }

  return NULL;
}

float determineAverageAngularDistanceM( struct Star arr[] )
{
  //making space for children processes
  struct tctl * tc = calloc(nthreads, sizeof(struct tctl));

  if (tc == NULL){
    perror("calloc");
    exit(1);
  }

  int i;
  
  //incase of error
  for (i = 0; i < nthreads; i++){
    tc[i].id = i;
    tc[i].arr = arr;
    int e = pthread_create(&tc[i].tid, NULL, tfun, tc + i);
    
    if (e){
      printf("thread_create: %s\n", strerror(e));
      exit(1);
    }
  }

  int e = pthread_join(tc[0].tid, NULL);

  if (e){
    printf("thread_join: %s\n", strerror(e));
    exit(1);
  }

  min = tc[0].min;
  max = tc[0].max;
  int count = tc[0].count;
  double total = tc[0].total;

  for (i = 1; i < nthreads; i++){
    int e = pthread_join(tc[i].tid, NULL);

    if (e){
      printf("thread_join2: %s\n", strerror(e));
      exit(1);
    }

    //finds minimum distance
    if (min > tc[i].min){
      min = tc[i].min;
    }

    //finds maximum distance
    if (max < tc[i].max){
      max = tc[i].max;
    }
    
    //calculates total distance
    count += tc[i].count;
    total += tc[i].total;
  }

  //calculates mean
  return total / count;  
}

int main( int argc, char * argv[] )
{

  FILE *fp;
  uint32_t star_count = 0;

  uint32_t n;

  if (argc > 1){
    nthreads = atoi(argv[1]);

    if (nthreads < 1){
      nthreads = 1;
    }
  }

  //allocate memory for distance calculations
  distance_calculated = malloc(sizeof(uint8_t[NUM_STARS][NUM_STARS]));

  if( distance_calculated == NULL )
  {
    uint64_t num_stars = NUM_STARS;
    uint64_t size = num_stars * num_stars * sizeof(uint8_t);
    printf("Could not allocate %ld bytes\n", size);
    exit( EXIT_FAILURE );
  }

  int i, j;

  for (i = 0; i < NUM_STARS; i++)
  {
    for (j = 0; j < NUM_STARS; j++)
    {
      distance_calculated[i][j] = 0;
    }
  }

  for( n = 1; n < argc; n++ )          
  {
    if( strcmp(argv[n], "-help" ) == 0 )
    {
      showHelp();
      exit(0);
    }
  }

  fp = fopen( "data/tycho-trimmed.csv", "r" );

  if( fp == NULL )
  {
    printf("ERROR: Unable to open the file data/tycho-trimmed.csv\n");
    exit(1);
  }

  char line[MAX_LINE];
  while (fgets(line, 1024, fp))
  {
    uint32_t column = 0;

    char* tok;
    for (tok = strtok(line, " ");
      tok && *tok;
      tok = strtok(NULL, " "))
    {
    switch( column )
      {
        case 0:
          star_array[star_count].ID = atoi(tok);
          break;
      
        case 1:
          star_array[star_count].RightAscension = atof(tok);
          break;
      
        case 2:
          star_array[star_count].Declination = atof(tok);
          break;

        default: 
          printf("ERROR: line %d had more than 3 columns\n", star_count );
          exit(1);
          break;
      }
       column++;
    }
    star_count++;
  }
  printf("%d records read\n", star_count );

  //determines the average angular distance
  //starts the timer
  time(NULL);
  double distance;
  
  //this function is called if multiple threads are used
  if (nthreads){
    distance =  determineAverageAngularDistanceM( star_array );
    time("Multithread time");
  }

  //this function is used if only one thread is used
  else {
    distance =  determineAverageAngularDistance( star_array );
    time("Single threaded time");
  }
  
  //print out data
  printf("Average distance found is %lf\n", distance );
  printf("Minimum distance found is %lf\n", min );
  printf("Maximum distance found is %lf\n", max );

  return 0;
}

