#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <iomanip>

#include <iostream>
#include <vector>
#include <map>
#include <algorithm>

#include "cache.cpp"
#include "sim.h"

/*  "argc" holds the number of command-line arguments.
    "argv[]" holds the arguments themselves.

    Example:
    ./sim 32 8192 4 262144 8 3 10 gcc_trace.txt
    argc = 9
    argv[0] = "./sim"
    argv[1] = "32"
    argv[2] = "8192"
    ... and so on
*/
int main (int argc, char *argv[]) {
    FILE *fp;			// File pointer.
    char *trace_file;		// This variable holds the trace file name.
    cache_params_t params;	// Look at the sim.h header file for the definition of struct cache_params_t.
    char rw;			// This variable holds the request's type (read or write) obtained from the trace.
    uint32_t addr;		// This variable holds the request's address obtained from the trace.
     			// The header file <inttypes.h> above defines signed and unsigned integers of various sizes in a machine-agnostic way.  "uint32_t" is an unsigned integer of 32 bits.

    // Exit with an error if the number of command-line arguments is incorrect.
    if (argc != 9) {
       printf("Error: Expected 8 command-line arguments but was provided %d.\n", (argc - 1));
       exit(EXIT_FAILURE);
    }
     
    // "atoi()" (included by <stdlib.h>) converts a string (char *) to an integer (int).
    params.BLOCKSIZE = (uint32_t) atoi(argv[1]);
    params.L1_SIZE   = (uint32_t) atoi(argv[2]);
    params.L1_ASSOC  = (uint32_t) atoi(argv[3]);
    params.L2_SIZE   = (uint32_t) atoi(argv[4]);
    params.L2_ASSOC  = (uint32_t) atoi(argv[5]);
    params.PREF_N    = (uint32_t) atoi(argv[6]);
    params.PREF_M    = (uint32_t) atoi(argv[7]);
    trace_file       = argv[8];

    // Open the trace file for reading.
    fp = fopen(trace_file, "r");
    if (fp == (FILE *) NULL) {
       // Exit with an error if file open failed.
       printf("Error: Unable to open file %s\n", trace_file);
       exit(EXIT_FAILURE);
    }
     
    // Print simulator configuration.
    printf("===== Simulator configuration =====\n");
    printf("BLOCKSIZE:  %u\n", params.BLOCKSIZE);
    printf("L1_SIZE:    %u\n", params.L1_SIZE);
    printf("L1_ASSOC:   %u\n", params.L1_ASSOC);
    printf("L2_SIZE:    %u\n", params.L2_SIZE);
    printf("L2_ASSOC:   %u\n", params.L2_ASSOC);
    printf("PREF_N:     %u\n", params.PREF_N);
    printf("PREF_M:     %u\n", params.PREF_M);
    printf("trace_file: %s\n", trace_file);
    printf("\n");

    //Creating two levels of Cache
    Cache l1_cache;
    l1_cache.set_block_size(params.BLOCKSIZE);
    l1_cache.set_size(params.L1_SIZE);
    l1_cache.set_assoc(params.L1_ASSOC);
    l1_cache.num_of_set();
    l1_cache.memory_map_init();
    
    Cache l2_cache;
    if(params.L2_SIZE != 0){ // create L2 cache only when L2_SIZE is not 0
    l2_cache.set_block_size(params.BLOCKSIZE);
    l2_cache.set_size(params.L2_SIZE);
    l2_cache.set_assoc(params.L2_ASSOC);
    l2_cache.num_of_set();
        l2_cache.memory_map_init();
    }
    

    if(params.PREF_N != 0){
        if(params.L2_SIZE != 0){
            l2_cache.set_prefetch_num(params.PREF_N);
            l2_cache.set_prefetch_size(params.PREF_M);
            l2_cache.prefetch_init();
        }
        else{
            l1_cache.set_prefetch_num(params.PREF_N);
            l1_cache.set_prefetch_size(params.PREF_M);
            l1_cache.prefetch_init();
        }
    }
         
    // Read requests from the trace file and echo them back.
    while (fscanf(fp, "%c %x\n", &rw, &addr) == 2) {	// Stay in the loop if fscanf() successfully parsed two tokens as specified.
        //if (rw == 'r')
        //    printf("r %x\n", addr);
        //else if (rw == 'w')
        //    printf("w %x\n", addr);
        //else {
        //    printf("Error: Unknown request type %c.\n", rw);
        //    exit(EXIT_FAILURE);
        //}

        ///////////////////////////////////////////////////////
        // Issue the request to the L1 cache instance here.
        ///////////////////////////////////////////////////////
        l1_cache.address_to_identifiers(addr);


        bool hit_in_l1_cache = l1_cache.get_address(rw);

        if(params.PREF_N != 0 && params.L2_SIZE == 0){
        l1_cache.search_stream_buffers(addr/params.BLOCKSIZE, hit_in_l1_cache, rw);
        }

        if(hit_in_l1_cache == false){ // send request to next level
            if(params.L2_SIZE != 0){
                bool hit_in_l2_cache;

                Cache::UpdateBlockStruct next_level = l1_cache.update_block(rw, addr);


                l2_cache.address_to_identifiers(next_level.addr);
                hit_in_l2_cache = l2_cache.get_address(next_level.rw);

                if(hit_in_l2_cache == false){
                    l2_cache.update_block(next_level.rw, next_level.addr); // update block in l2
                    if(next_level.rw == 'r'){
                        l2_cache.read_misses_next_level++;
                    
                    }
                }
                if(params.PREF_N != 0){
                    l2_cache.search_stream_buffers(next_level.addr/params.BLOCKSIZE, hit_in_l2_cache, next_level.rw);
                }
                

                if(next_level.dirty == true){
                    l2_cache.address_to_identifiers(addr);
                    hit_in_l2_cache = l2_cache.get_address('r');
                    if(hit_in_l2_cache == false){
                        l2_cache.update_block('r', addr); // update block in l2
                        l2_cache.read_misses_next_level++;
                    }
                     if(params.PREF_N != 0){
                        l2_cache.search_stream_buffers(addr/params.BLOCKSIZE, hit_in_l2_cache, 'r');
                    }
                }

            }
            else{
                l1_cache.update_block(rw, addr);// update block in l1
                                               // main memory counter here
            }
        }
   
    }

    std::cout << "===== L1 contents =====" << std::endl;
    l1_cache.print_memory_map();
    
    if(params.L2_SIZE != 0){
    std::cout << "===== L2 contents =====" << std::endl;
    l2_cache.print_memory_map();
    }
    
    if(params.PREF_N != 0){
        if(params.L2_SIZE != 0){
            l2_cache.print_prefetch_map();
        }
        else{
            l1_cache.print_prefetch_map();
        }
    }

    float l1_miss_rate = static_cast<float>(l1_cache.read_misses - l1_cache.num_read_prefetch_hit + l1_cache.write_misses - l1_cache.num_write_prefetch_hit)/(l1_cache.read_req + l1_cache.write_req);
    float l2_miss_rate = 0;

    int memory_traffic = l1_cache.read_misses - l1_cache.num_read_prefetch_hit + l1_cache.write_misses - l1_cache.num_write_prefetch_hit + l1_cache.writebacks_next_level + l1_cache.num_prefetch;

    if(params.L2_SIZE !=0){
        l2_miss_rate = static_cast<float>(l2_cache.read_misses - l2_cache.num_read_prefetch_hit)/l2_cache.read_req;
        memory_traffic = l2_cache.read_misses - l2_cache.num_read_prefetch_hit + 0 + l2_cache.write_misses - l2_cache.num_write_prefetch_hit + l2_cache.writebacks_next_level + l2_cache.num_prefetch;
    }

    std::cout << "===== Measurements =====" << std::endl;
    std::cout << "a. L1 reads:                   " << l1_cache.read_req << std::endl;
    std::cout << "b. L1 read misses:             " << l1_cache.read_misses - l1_cache.num_read_prefetch_hit << std::endl; 
    std::cout << "c. L1 writes:                  " << l1_cache.write_req << std::endl;
    std::cout << "d. L1 write misses:            " << l1_cache.write_misses - l1_cache.num_write_prefetch_hit << std::endl;
          printf("e. L1 miss rate:               %0.4f\n", l1_miss_rate);
    std::cout << "f. L1 writebacks:              " << l1_cache.writebacks_next_level << std::endl;
    std::cout << "g. L1 prefetches:              " << l1_cache.num_prefetch << std::endl;
    std::cout << "h. L2 reads (demand):          " << l2_cache.read_req << std::endl;
    std::cout << "i. L2 read misses (demand):    " << l2_cache.read_misses - l2_cache.num_read_prefetch_hit << std::endl;
    std::cout << "j. L2 reads (prefetch):        " << 0 << std::endl;
    std::cout << "k. L2 read misses (prefetch):  " << 0 << std::endl;
    std::cout << "l. L2 writes:                  " << l2_cache.write_req << std::endl;
    std::cout << "m. L2 write misses:            " << l2_cache.write_misses - l2_cache.num_write_prefetch_hit << std::endl;
          printf("n. L2 miss rate:               %0.4f\n", l2_miss_rate);
    std::cout << "o. L2 writebacks:              " << l2_cache.writebacks_next_level << std::endl;
    std::cout << "p. L2 prefetches:              " << l2_cache.num_prefetch << std::endl;
    std::cout << "q. memory traffic:             " << memory_traffic << std::endl;

    return(0);
}
