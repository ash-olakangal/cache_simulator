#include "cache.h"

int Cache::hit_counter = 0;
int Cache::miss_counter = 0;

void Cache::set_size(int sim_size){
    cache_size = sim_size;
    //std::cout << "CACHE SIZE = " << cache_size << std::endl;
}

void Cache::set_block_size(int sim_block_size){
    block_size = sim_block_size;
    //std::cout << "CACHE BLOCK SIZE = " << block_size << std::endl;
}

void Cache::set_assoc(int sim_assoc){
    assoc = sim_assoc;
    //std::cout << "CACHE ASSOC = " << assoc << std::endl;
}

void Cache::set_prefetch_num(int sim_prefetch_num){
    prefetch_num = sim_prefetch_num;
    //std::cout << "PREFETCH NUM = " << prefetch_num << std::endl;
}

void Cache::set_prefetch_size(int sim_prefetch_size){
    prefetch_size = sim_prefetch_size;
    //std::cout << "PREFETCH SIZE = " << prefetch_size << std::endl;
}

void Cache::num_of_set(){
    num_set = (cache_size/(assoc*block_size));
    //std::cout << "NUM_SET = " << num_set << std::endl;
}

void Cache::address_to_identifiers( uint32_t decimal_address){
    //unsigned long long decimal_address = std::stoull(hex_address, nullptr, 16);

    //std::cout << "Address = " << decimal_address << std::endl;

    // calculating block offset
    current_block_offset = decimal_address%block_size;
    //std::cout << "current_block_offset = " << current_block_offset << std::endl;

    // removing the offset value
    decimal_address = decimal_address/block_size;
    //std::cout << "removing offset value: Address = " << decimal_address << std::endl;

    // calculating index
    current_index = decimal_address%num_set;
    //std::cout << "current_index = " << current_index << std::endl;

    // removing index value
    decimal_address = decimal_address/num_set;
    //std::cout << "removing index value: Address = " << decimal_address << std::endl;

    current_address_tag = decimal_address; // remaining value will be tag
    //std::cout << "current_address_tag = " << current_address_tag << std::endl;
}

uint32_t Cache::identifiers_to_address() {
    uint32_t address = 0;

    // Start with the tag
    address = current_address_tag;

    // Add the index (reversing the division by num_set)
    address = (address * num_set) + current_index;

    // Add the block offset (reversing the division by block_size)
    address = (address * block_size) + current_block_offset;

    return address;
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

void Cache::print_memory_map() {
    for (auto const& pair : memory_map) {
        bool has_valid_block = false;
        // Check for at least one valid block in the set
        for (const auto& transaction : pair.second) {
            if (transaction.valid == 1) {
                has_valid_block = true;
                break;
            }
        }

        if (has_valid_block) {
            // Create a temporary copy of the vector to sort it
            std::vector<BlockStruct> sorted_blocks = pair.second;

            // Sort the blocks by lru_count in descending order
            std::sort(sorted_blocks.begin(), sorted_blocks.end(), 
                [](const BlockStruct& a, const BlockStruct& b) {
                    return a.lru_count > b.lru_count;
                });

            // Print the set number
            std::cout << "set\t" << pair.first << ":\t";
            
            // Print the sorted blocks
            for (const auto& transaction : sorted_blocks) {
                if (transaction.valid == 0) {
                    continue; // Skip invalid blocks
                }
                
                std::cout << std::hex << transaction.tag;
                std::cout << std::dec; // Switch back to decimal

                if (transaction.dirty) {
                    std::cout << " D\t";
                } else {
                    std::cout << "  \t";
                }
            }
            std::cout << std::endl;
        }
    }
    std::cout << std::endl;
}

bool Cache::get_address(char rw){

    for(int block=0; block<assoc; block++){
        BlockStruct get_mem_block = memory_map[current_index][block];
        if(get_mem_block.valid == 1 && get_mem_block.tag == current_address_tag){
            hit_counter++;

            //std::cout << "HIT COUNTER: " << hit_counter << std::endl;
            
            for(int block_lru_update=0; block_lru_update<assoc; block_lru_update++){
                if(memory_map[current_index][block_lru_update].lru_count > get_mem_block.lru_count){

                // if count < hit_count -> stay the same
                // if count > hit_count -> decrement by 1
                memory_map[current_index][block_lru_update].lru_count -= 1;
                }
            }
            if(memory_map[current_index][block].dirty == 0 && rw == 'w'){
                memory_map[current_index][block].dirty = 1;
            }
            
            memory_map[current_index][block].lru_count = assoc-1; // lru_count = MAX for MRU
            return true;
        }
    }

    miss_counter++;
    //std::cout << "MISS COUNTER: " << miss_counter << std::endl;

    return false;
    //update_block();
}

uint32_t Cache::update_block(char rw, uint32_t addr){
    
    //std::cout << "Updating the block" << std::endl;

    uint32_t replaced_address = addr;

    for(int block = 0; block < assoc; block++){
       
        // lru_count = 0 for lru
        // lru_count = MAX(=assoc-1) for mru
        if(memory_map[current_index][block].lru_count == 0){

            if(memory_map[current_index][block].dirty == 1){
               replaced_address = identifiers_to_address(); // to be given to next level of cache
                memory_map[current_index][block].dirty = 0;
            }

            memory_map[current_index][block].valid = 1;
            memory_map[current_index][block].tag = current_address_tag;
            memory_map[current_index][block].lru_count = assoc-1;

            // WB policy
            if(rw == 'w'){
            memory_map[current_index][block].dirty = 1;
            }

        }
        else{
            memory_map[current_index][block].lru_count--;
        }
    }
    return replaced_address;
}

// TODO: Update the WBWA policy:
// wr request to next level block followed by read request if dirty = 1
// only read request to next level cache if dirty = 0 or valid = 0
// no requests to next level cache if hit in current level cache

void Cache::prefetch_buffer_init(){
    for(int block_num=0; block_num<prefetch_size; block_num++){
        PrefetchStruct prefetch_init_block; // ISSUE TODO
        prefetch_init_block.address = 0;
        prefetch_init_block.valid = 0;

        for(int i=0; i<prefetch_num; i++){
        prefetch_init_block.lru_count = i;
        prefetch_memory_map[i].push_back(prefetch_init_block);
        }
    }
    
}

void Cache::print_prefetch_map(){
    auto iter = prefetch_memory_map.begin();

    while(iter != prefetch_memory_map.end()) {
        std::cout << "Buffer: " << iter->first << std::endl;
        for (const auto& transaction : iter->second) {
            std::cout << "\taddress: " << transaction.address
                      << ", valid: " << transaction.valid
                      << ", lru_count: " << transaction.lru_count << std::endl;
        }
        iter++;
    }
}


void Cache::prefetch_stream(uint32_t address){
   // get next 'M' addresses from main-memory when miss in cache
   // init case - fill the buffer
    for(int i=0; i<prefetch_num; i++){
        for(int j=0; j<prefetch_size; j++){
            if(prefetch_memory_map[i][j].valid == 0 && prefetch_memory_map[i][j].lru_count == 0){ //lru_count = 0 - LRU, MAX - MRU 
                prefetch_memory_map[i][j].address = address+j;            
                prefetch_memory_map[i][j].lru_count = prefetch_num-1;
            }
            else{
                prefetch_memory_map[i][j].lru_count--;
            }
        }   
    }
    //for loop -> prefetch_size
    //prefetch_memory_map[i].pop();
    //prefetch_memory_map[i].push();

    //for loop -> prefetch_size
    //if(address exists in buffer)
    //    //pop all including matched block 
    //else
    //    return
    // pop case - pop the matches and push the next n addresses

}
