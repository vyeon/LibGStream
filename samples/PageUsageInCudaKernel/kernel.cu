#include <gstream/cuda/datatype/device_slotted_page.h>
#include <gstream/datatype/pagedb.h>
#include <cuda_runtime.h>

// Define meta parameter for page type 
using vertex_id_t = uint8_t;
using page_id_t = uint8_t;
using record_offset_t = uint8_t;
using slot_offset_t = uint8_t;
using record_size_t = uint8_t;
using edge_payload_t = uint8_t;
using vertex_payload_t = uint8_t;
constexpr std::size_t PageSize = 64;

// Define page type
using page_t = gstream::slotted_page<vertex_id_t, page_id_t, record_offset_t, slot_offset_t, record_size_t, PageSize, edge_payload_t, vertex_payload_t>;
// Define device page type
using device_page_t = gstream::device_slotted_page_t<page_t>;

__global__ void kernel(device_page_t* pages, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i) {
        printf("@ page[%llu]--------------------------------------\n", i);
        printf("page type: %s\n",
            (pages[i].is_sp()) ?
               "small page" : (pages[i].is_lp_head()) ?
               "large page (head)" : "large page (extended)");
        
        std::uint64_t number_of_slots = pages[i].number_of_slots();
        printf("number of slots in the page: %llu\n", number_of_slots);
        for (std::size_t j = 0; j < number_of_slots; ++j)
            printf("- slot[%llu]\tVID: %u\tRec-OFF: %u\tV-PL: %u\n", 
                   j,
                   pages[i].slot(j).vertex_id,
                   pages[i].slot(j).record_offset,
                   pages[i].slot(j).vertex_payload );
        printf("\n");
    }
}

int main()
{
    // Read pages from file using container as vector
    auto pages = gstream::read_pages<page_t, std::vector>("wewv.pages");
    
    // Initialize: Allocate device buffer
    void* devbuf;
    cudaMalloc(&devbuf, sizeof(page_t) * pages.size());
    // Copy host pages to device buffer
    cudaMemcpy(devbuf, pages.data(), sizeof(page_t) * pages.size(), cudaMemcpyHostToDevice);
    // Call kernel
    kernel <<< 1, 1 >>>(reinterpret_cast<device_page_t*>(devbuf), pages.size());
    
    // Finalize resources
    cudaFree(devbuf);
    cudaDeviceSynchronize();
    cudaDeviceReset();

	return 0;
}