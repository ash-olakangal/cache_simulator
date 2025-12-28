#ifndef CACHE_H
#define CACHE_H

class Cache{
    private:

        int next_lru_tick = 0;
        // cache attributes
        int cache_size;
        int block_size;
        int assoc;
        int num_set; 
        int prefetch_num=0;
        unsigned int prefetch_size=0;
        
        // identifier bits
        int current_index;
        int current_block_offset;
        int current_address_tag;

        char next_level;

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

        struct UpdateBlockStruct {
        char rw;
        uint32_t addr;
        bool dirty;
        };

        void set_size(int);
        void set_block_size(int);
        void set_assoc(int);
        void set_prefetch_num(int);
        void set_prefetch_size(int);

        void num_of_set();

        void address_to_identifiers(uint32_t);
        uint32_t identifiers_to_address(int, int);

        void display_identifiers();

        void memory_map_init();
        void print_memory_map();

        bool get_address(char);
        Cache::UpdateBlockStruct update_block(char, uint32_t);

        void prefetch_init();
        void print_prefetch_map();
        bool search_stream_buffers(uint32_t, bool, char);

        //statistics variables
        int read_req=0;
        int read_misses_next_level=0;
        int read_misses=0;
        int write_req=0;
        int write_misses=0;
        int writebacks_next_level=0;
        int num_prefetch=0;
        int num_read_prefetch_hit=0;
        int num_write_prefetch_hit=0;
        int memory_req=0;
};

#endif
