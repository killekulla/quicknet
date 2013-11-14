#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>


#include "networknode.h"
#include "bstreap.h"
#include "heap.h"
#include "krapivsky.h"
// quickmath also defines bufsize, so get to it first
#ifndef BUFSIZE
#define BUFSIZE 1024
#endif
#include "quickmath.h"

int main(int argc, char **argv) {
  // counter
  uint64_t i;
  // arg placeholder for getopt
  int c;
  // timing variables
  clock_t t1, t2;

  // simulation input
  krapivsky_input_t *input;
  // model data structure
  krapivsky_model_t *km;
  
  // model parameters
  double p = 0.2;
  double lambda = 3.5;
  double mu = 1.8;
  
  // number of nodes to simulate
  uint64_t target_n_nodes = 1000;
  
  // number of simulations to run
  uint64_t n_runs = 1;

  // base output dir
  char base_dir_name[BUFSIZE] = ".";
  
  // edge output filename
  char edge_file_name[BUFSIZE] = "edges.csv";
  char edge_file_fullname[BUFSIZE];
  // write out the network's edges?
  int write_edges = 0;
  
  // output filename for each run, constructed automatically
  char run_file_fullname[BUFSIZE];
  // output filename for timing data
  char time_file_name[BUFSIZE] = "times.csv";
  char time_file_fullname[BUFSIZE];
  FILE *time_file;

  // random seed filename
  char random_seed_file_name[BUFSIZE] = "randomseed.csv";
  
  // write out timing information?
  int write_time = 0;
  // array of execution times
  double *times;
  
  // the type of data structure to use for node indexing
  char type[BUFSIZE] = "heap";
  
  // parse command line args
  while((c = getopt(argc, argv, "s:e:t:p:m:l:n:o:r:u:")) != -1){
    switch(c) {
    case 's':
      strcpy(random_seed_file_name, optarg);
      break;
    case 'e':
      strcpy(edge_file_name, optarg);
      write_edges = 1;
      break;
    case 't':
      strcpy(type, optarg);
      break;
    case 'p':
      sscanf(optarg, "%lf", &p);
      break;
    case 'm':
      sscanf(optarg, "%lf", &mu);
      break;
    case 'l':
      sscanf(optarg, "%lf", &lambda);
      break;
    case 'n':
      sscanf(optarg, "%llu", &target_n_nodes);
      break;
    case 'o':
      strcpy(base_dir_name,optarg);
      break;
    case 'r':
      sscanf(optarg, "%llu", &n_runs);
      break;
    case 'u':
      write_time = 1;
      strcpy(time_file_name,optarg);
      break;
    }
  }

  // prepend basedir to filenames
  sprintf(edge_file_fullname, "%s/%s", base_dir_name, edge_file_name);
  sprintf(time_file_fullname, "%s/%s", base_dir_name, time_file_name);

  // initialize pseudorandom number generator
  rand_init(base_dir_name, random_seed_file_name);

  // open the time file if we'll be using it
  if(write_time) {
    times = (double*) malloc(n_runs * sizeof(*times));
    time_file = fopen(time_file_fullname,"w");
  }

  // construct the simulation input
  input = krapivsky_make_input(p, lambda, mu, target_n_nodes);
  
  // run the simulations
  for(i=0;i<n_runs;i++) {
    t1 = clock();
    if(strcmp(type,"lnu") == 0) {
      km = krapivsky_bstreap_simulate_lnu(input);
    } else if(strcmp(type,"lnn") == 0) {
      km = krapivsky_bstreap_simulate_lnn(input);
    } else if(strcmp(type,"lsu") == 0) {
      km = krapivsky_bstreap_simulate_lsu(input);
    } else if(strcmp(type,"lsn") == 0) {
      km = krapivsky_bstreap_simulate_lsn(input);
    } else if(strcmp(type,"heap") == 0) {
      km = krapivsky_heap_simulate(input);
    } else if(strcmp(type,"lnupareto") == 0) {
      km = krapivsky_bstreap_simulate_pareto_lnu(input);
    } else if(strcmp(type,"lnnpareto") == 0) {
      km = krapivsky_bstreap_simulate_pareto_lnn(input);
    } else if(strcmp(type,"lsupareto") == 0) {
      km = krapivsky_bstreap_simulate_pareto_lsu(input);
    } else if(strcmp(type,"lsnpareto") == 0) {
      km = krapivsky_bstreap_simulate_pareto_lsn(input);
    } else if(strcmp(type,"heappareto") == 0) {
      km = krapivsky_heap_simulate_pareto(input);
    } else if(strcmp(type,"heapquadratic") == 0) {
      km = krapivsky_heap_simulate_quadratic(input);
    } else {
      printf("Unknown type: %s\n",type);
      return 1;
    }
    t2 = clock();
    if(write_edges) {
      sprintf(run_file_fullname,
              "%s_%llu",
              edge_file_fullname,
              i);
      krapivsky_write_edges(km,run_file_fullname);
    }
    if(write_time) {
      times[i] = ((double) (t2 - t1))/CLOCKS_PER_SEC;  //time in seconds
      fprintf(time_file, "%lf\n", times[i]);  
    }
    krapivsky_free(km);
  }
  free(input);

  // output and clean up timing data
  if(write_time) {
    printf("Simulated %llu networks with %llu nodes each.\nEach network took %lf +/- %lf seconds to simulate.\n",
           n_runs,
           target_n_nodes,
           mean(times,n_runs),
           stdev(times,n_runs));
    free(times);
    fclose(time_file);
  }
  return 0;
}
