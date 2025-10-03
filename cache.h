#ifndef CACHE_H
#define CACHE_H

class Cache{
    private:
        // cache attributes
        int cache_size;
        int block_size;
        int assoc;
        int num_set; 
        
        // identifier bits
        int current_index;
        int current_block_offset;
        int current_address_tag;

        //counters
        static int hit_counter;
        static int miss_counter;

        struct BlockStruct{
            int valid;
            int dirty;
            int lru_count;
            int tag;
        };

        std::unordered_map<int, std::vector<BlockStruct>> memory_map;

    public:
        void set_size(int);
        void set_block_size(int);
        void set_assoc(int);
        void num_of_set();

        void address_to_identifiers(std::string);

        void display_identifiers();

        void memory_map_init();
        void print_memory_map();

        bool get_address();
        void update_block();
};

#endif
