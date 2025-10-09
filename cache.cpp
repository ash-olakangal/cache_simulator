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

void Cache::prefetch_init(){
    stream_buffers.resize(prefetch_num);
    int count=0;
    for (auto& buffer : stream_buffers) {
        buffer.valid = false;
        buffer.addresses.resize(prefetch_size);
        buffer.lru_count = count;
        count++;
    }
}
    
void Cache::print_prefetch_map(){

    std::vector<StreamBuffer> sorted_buffers = stream_buffers;
    
    // Sort the buffers by lru_count in descending order (MRU first).
    std::sort(sorted_buffers.begin(), sorted_buffers.end(), 
        [](const StreamBuffer& a, const StreamBuffer& b) {
            return a.lru_count > b.lru_count;
        });

    std::cout << "===== Stream Buffer(s) contents =====" << std::endl;
    
    for (const auto& buffer : sorted_buffers) {
        if (buffer.valid) {
            for (const auto& address : buffer.addresses) {
                std::cout << std::hex << " " << address << " ";
            }
            std::cout << std::endl;
        }
    }
}


bool Cache::search_stream_buffers(uint32_t target_address) {
    // Keep track of the maximum lru_count for updating later
    //std::cout << std::hex << target_address <<std::endl;
    int max_lru_count = 0;
    
    // Sort a temporary vector of pointers to original buffers by recency
    std::vector<StreamBuffer*> sorted_buffer_ptrs;
    for (auto& buffer : stream_buffers) {
        sorted_buffer_ptrs.push_back(&buffer);
        if (buffer.valid && buffer.lru_count > max_lru_count) {
            max_lru_count = buffer.lru_count;
        }
    }
    std::sort(sorted_buffer_ptrs.begin(), sorted_buffer_ptrs.end(),
        [](const StreamBuffer* a, const StreamBuffer* b) {
            return a->lru_count > b->lru_count;
        });

    // 1. Search for a hit in existing valid buffers
    for (StreamBuffer* buffer_ptr : sorted_buffer_ptrs) {
        if (!buffer_ptr->valid) continue;
        
        auto& addresses = buffer_ptr->addresses;
        auto it = std::find(addresses.begin(), addresses.end(), target_address);
        if (it != addresses.end()) {
            // Hit found, update buffer
            uint32_t last_address_in_buffer = 0;
            if (it + 1 != addresses.end()) {
                last_address_in_buffer = *(it + 1);
            } else {
                last_address_in_buffer = *it;
            }
            addresses.erase(addresses.begin(), it + 1);
            size_t num_to_refill = prefetch_size - addresses.size();
            for (size_t i = 0; i < num_to_refill; ++i) {
                addresses.push_back(last_address_in_buffer + i);
                //std::cout << std:: hex << "STREAM BUFFER_ADDR: " << last_address_in_buffer+i <<std::endl;
            }
            
            // Update LRU counts
            buffer_ptr->lru_count = max_lru_count + 1;
            
            return true; // Hit
        }
    }
    
    // 2. No hit, so handle a new fetch
    
    // Find an invalid buffer first
    for (StreamBuffer& buffer : stream_buffers) {
        if (!buffer.valid) {
            buffer.lru_count = max_lru_count + 1;
            buffer.valid = true;
            buffer.addresses.clear();
            for (int i = 0; i < prefetch_size; ++i) {
                buffer.addresses.push_back(target_address + i + 1);
            //std::cout << std::hex << "STREAM BUFFER_INVALID_ADDR: " << target_address + i << std::endl;
            }
            return false; // Miss, but a buffer was available
        }
    }
    
    // 3. No hit and no invalid buffers, so replace LRU buffer
    StreamBuffer* lru_buffer = sorted_buffer_ptrs.back();
    lru_buffer->lru_count = max_lru_count + 1;
    lru_buffer->valid = true;
    lru_buffer->addresses.clear();
    for (int i = 0; i < prefetch_size; ++i) {
        lru_buffer->addresses.push_back(target_address + i + 1);
        //std::cout << std::hex << "STREAM BUFFER_REPL_ADDR: " << target_address + i << std::endl;
    }
    return false; // Miss, LRU buffer replaced
}


