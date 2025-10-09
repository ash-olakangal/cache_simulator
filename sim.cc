#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

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

        if(params.PREF_N != 0 && params.L2_SIZE != 0){
        l1_cache.search_stream_buffers(addr/params.BLOCKSIZE, hit_in_l1_cache);
        }

        if(hit_in_l1_cache == false){ // send request to next level
            if(params.L2_SIZE != 0){
                bool hit_in_l2_cache;

                Cache::UpdateBlockStruct next_level = l1_cache.update_block(rw, addr);


                l2_cache.address_to_identifiers(next_level.addr);
                hit_in_l2_cache = l2_cache.get_address(next_level.rw);

                if(hit_in_l2_cache == false){
                    l2_cache.update_block(next_level.rw, next_level.addr); // update block in l2
                }
                if(params.PREF_N != 0){
                    l2_cache.search_stream_buffers(next_level.addr/params.BLOCKSIZE, hit_in_l2_cache);
                }
                

                if(next_level.dirty == true){
                    l2_cache.address_to_identifiers(addr);
                    hit_in_l2_cache = l2_cache.get_address('r');
                    if(hit_in_l2_cache == false)
                    l2_cache.update_block('r', addr); // update block in l2
                     if(params.PREF_N != 0){
                        l2_cache.search_stream_buffers(addr/params.BLOCKSIZE, hit_in_l2_cache);
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

    return(0);
}
