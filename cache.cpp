#include "cache.h"

int Cache::hit_counter = 0;
int Cache::miss_counter = 0;

void Cache::set_size(int sim_size){
    cache_size = sim_size;
    std::cout << "CACHE SIZE = " << cache_size << std::endl;
}

void Cache::set_block_size(int sim_block_size){
    block_size = sim_block_size;
    std::cout << "CACHE BLOCK SIZE = " << block_size << std::endl;
}

void Cache::set_assoc(int sim_assoc){
    assoc = sim_assoc;
    std::cout << "CACHE ASSOC = " << assoc << std::endl;
}

void Cache::num_of_set(){
    num_set = (cache_size/(assoc*block_size));
    std::cout << "NUM_SET = " << num_set << std::endl;
}

void Cache::address_to_identifiers(std::string hex_address){
    unsigned long long decimal_address = std::stoull(hex_address, nullptr, 16);

    std::cout << "Address = " << decimal_address << std::endl;

    // calculating block offset
    current_block_offset = decimal_address%block_size;

    // removing the offset value
    decimal_address = decimal_address/block_size;

    // calculating index
    current_index = decimal_address%num_set;

    // removing index value
    decimal_address = decimal_address/num_set;

    current_address_tag = decimal_address; // remaining value will be tag
}

void Cache::display_identifiers(){
    std::cout << "Block offset = " << current_block_offset << std::endl;
    std::cout << "Index = " << current_index << std::endl;
    std::cout << "Tag = " << current_address_tag << std::endl;
}

void Cache::memory_map_init(){

    // initializing memory map
    for(int set=0; set<num_set; set++){
        BlockStruct init_block;
        init_block.valid = 0;
        init_block.dirty = 0;
        init_block.tag = 0;

        for(int block=0; block<assoc; block++){
            init_block.lru_count = block;
            memory_map[set].push_back(init_block);
        }
    }
}

void Cache::print_memory_map(){
    auto iter = memory_map.begin();

    while(iter != memory_map.end()) {
        std::cout << "Set: " << iter->first << std::endl;
        for (const auto& transaction : iter->second) {
            std::cout << "\tvalid: " << transaction.valid
                      << ", dirty: " << transaction.dirty
                      << ", lru_count: " << transaction.lru_count
                      << ", tag: " << transaction.tag << std::endl;
        }
        iter++;
    }
}

bool Cache::get_address(){
    
    for(int block=0; block<assoc; block++){
        BlockStruct get_mem_block = memory_map[current_index][block];
        if(get_mem_block.valid == 1 && get_mem_block.tag == current_address_tag){
            hit_counter++;
            std::cout << "HIT COUNTER: " << hit_counter << std::endl;
            
            for(int block_lru_update=0; block_lru_update<assoc; block_lru_update++){
                if(memory_map[current_index][block_lru_update].lru_count > get_mem_block.lru_count){

                // if count < hit_count -> stay the same
                // if count > hit_count -> decrement by 1
                memory_map[current_index][block_lru_update].lru_count -= 1;
                }
            }
            memory_map[current_index][block].lru_count = assoc-1; // lru_count = MAX for MRU
            return true;
        }
    }

    miss_counter++;
    std::cout << "MISS COUNTER: " << miss_counter << std::endl;
    return false;
    //update_block();
}

void Cache::update_block(){
    
    std::cout << "Updating the block" << std::endl;

    for(int block = 0; block < assoc; block++){
       
        // lru_count = 0 for lru
        // lru_count = MAX(=assoc-1) for mru
        if(memory_map[current_index][block].lru_count == 0){
            memory_map[current_index][block].valid = 1;
            memory_map[current_index][block].tag = current_address_tag;
            memory_map[current_index][block].lru_count = assoc-1;
            // WB policy
            memory_map[current_index][block].dirty = 1;
        }else{
            memory_map[current_index][block].lru_count--;
        }
    }
}

// TODO: Update the WBWA policy:
// wr request to next level block followed by read request if dirty = 1
// only read request to next level cache if dirty = 0 or valid = 0
// no requests to next level cache if hit in current level cache
