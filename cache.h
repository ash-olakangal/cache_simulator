#ifndef CACHE_H
#define CACHE_H

class Cache{
    private:
        // cache attributes
        int cache_size;
        int block_size;
        int assoc;
        int num_set; 
        int prefetch_num;
        int prefetch_size;
        
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

        struct PrefetchStruct{
            int address;
            int valid;
            int lru_count;
        };

        std::map<int, std::vector<BlockStruct>> memory_map;
        std::map<int, std::vector<PrefetchStruct>> prefetch_memory_map;

    public:
        void set_size(int);
        void set_block_size(int);
        void set_assoc(int);
        void set_prefetch_num(int);
        void set_prefetch_size(int);

        void num_of_set();

        void address_to_identifiers(uint32_t);
        uint32_t identifiers_to_address();

        void display_identifiers();

        void memory_map_init();
        void print_memory_map();

        bool get_address(char);
        uint32_t update_block(char, uint32_t);

        void prefetch_buffer_init();
        void print_prefetch_map();
        void prefetch_stream(uint32_t);
};

#endif
