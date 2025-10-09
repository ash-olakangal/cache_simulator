#ifndef CACHE_H
#define CACHE_H

class Cache{
    private:
        // cache attributes
        int cache_size;
        int block_size;
        int assoc;
        int num_set; 
        int prefetch_num=0;
        int prefetch_size=0;
        
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

        struct StreamBuffer {
            bool valid = false; // Initially invalid
            int lru_count;
            std::vector<uint32_t> addresses;
        };

        std::map<int, std::vector<BlockStruct>> memory_map;
        std::vector<StreamBuffer> stream_buffers;
        
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

        void prefetch_init();
        void print_prefetch_map();
        bool search_stream_buffers(uint32_t);
};

#endif
