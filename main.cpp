#include <iostream>
#include <vector>
#include <unordered_map>

#include "cache.cpp"

int main(){
    Cache my_cache;
    //my_cache.set_size(512);
    my_cache.set_size(256);
    my_cache.set_block_size(16);
    my_cache.set_assoc(4);
    //my_cache.set_assoc(1);
    my_cache.num_of_set();
    my_cache.address_to_identifiers("FFE04540");
    my_cache.display_identifiers();
    my_cache.memory_map_init();
    my_cache.print_memory_map();
    my_cache.get_address();
    my_cache.update_block();
    my_cache.print_memory_map();
    my_cache.get_address();
    my_cache.address_to_identifiers("DEAD4740");
    my_cache.get_address();
    my_cache.update_block();
    my_cache.print_memory_map();
    my_cache.address_to_identifiers("FFE04540");
    my_cache.get_address();
    my_cache.print_memory_map();
}
