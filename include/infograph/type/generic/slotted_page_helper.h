/* ---------------------------------------------------------------
**
** InfoGraph - InfoLab Graph Library
**
** slotted_page_helper.h
** helper functions for generic slotted_page class
**
** Author: Seyeon Oh (vee@dgist.ac.kr)
** ------------------------------------------------------------ */

#ifndef _INFOGRAPH_TYPE_GENERIC_SLOTTED_PAGE_HELPER_
#define _INFOGRAPH_TYPE_GENERIC_SLOTTED_PAGE_HELPER_

#include <infograph/type/generic/slotted_page.h>
#include <infograph/mpl.h>

#ifdef _WIN32
#include <conio.h>
#include <Windows.h>
#endif // !_WIN32

namespace igraph {

template <int PRINT_CARRIAGE_RETURN_THRESHOLD = 64, class PAGE_T>
void print_page(const PAGE_T& page)
{
#ifndef _WIN32

#else // _WIN32 defined
    using color_t = short;
    auto set_color = [](color_t color) -> bool
    {
        return !!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
    };
    auto get_color = [](color_t &ret) -> bool
    {
        CONSOLE_SCREEN_BUFFER_INFO info;
        if (!GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
            return false;
        ret = info.wAttributes;
        return true;
    };

    color_t cdef_origin = 0;
    if (!get_color(cdef_origin))
        cdef_origin = 0x07;

    size_t  g_offset = 0;
    const auto number_of_slots = page.number_of_slots();
    const color_t cdef_adj_page_id[2] = { 0x1A, 0x4A };
    const color_t cdef_adj_offset[2] = { 0x1B, 0x4B };
    const color_t cdef_vertex_id[2] = { 0x6E, 0x5E };
    const color_t cdef_internal_offset[2] = { 0x6F, 0x5F };
    const color_t cdef_adj_list_size[2] = { 0x2F, 0x3F };
    const color_t cdef_payload[2] = { 0x1E , 0x4E };
    const color_t cdef_footer_padding = 0x70;
    const color_t cdef_footer_flag = 0x8F;
    const color_t cdef_footer_front = 0x70;
    const color_t cdef_footer_rear = 0xF0;
    const color_t cdef_unused = 0x08;

    auto print_byte = [&](uint8_t data)
    {
        printf("%02X", data);
        if (g_offset % PRINT_CARRIAGE_RETURN_THRESHOLD == 0)
        {
            color_t old = 0;
            get_color(old);
            set_color(cdef_origin);
            printf("|\n");
            set_color(old);
        }
        else
        {
            printf(" ");
        }
    };

    if (number_of_slots == 0)
    {
        set_color(cdef_unused);
        while (g_offset < PAGE_T::DATA_SECTION_SIZE)
            print_byte(page.data_section[g_offset++]);
    }
    else if (page.footer.flags & page_flag::SMALL_PAGE)
    {
        for (typename PAGE_T::offset_t slot_offset = 0;
             slot_offset < number_of_slots;
             ++slot_offset)
        {
            set_color(cdef_adj_list_size[slot_offset % 2]);
            for (size_t idx = 0; idx < sizeof(PAGE_T::adj_list_size_t); ++idx)
                print_byte(page.data_section[g_offset++]);
            size_t list_size = page.adj_list_size(slot_offset);
            for (size_t idx = 0; idx < list_size; ++idx)
            {
                set_color(cdef_adj_page_id[idx % 2]);
                for (size_t _idx = 0; _idx < sizeof(PAGE_T::adj_page_id_t); ++_idx)
                    print_byte(page.data_section[g_offset++]);
                set_color(cdef_adj_offset[idx % 2]);
                for (size_t _idx = 0; _idx < sizeof(PAGE_T::adj_offset_t); ++_idx)
                    print_byte(page.data_section[g_offset++]);
                set_color(cdef_payload[idx % 2]);
                for (size_t _idx = 0; _idx < PAGE_T::adj_payload_size; ++_idx)
                    print_byte(page.data_section[g_offset++]);
            }
        }
    }
    else if (page.footer.flags & page_flag::LARGE_PAGE)
    {
        if (page.footer.flags & page_flag::LEAD_PAGE)
        {
            set_color(cdef_adj_list_size[0]);
            for (size_t idx = 0; idx < sizeof(PAGE_T::adj_list_size_t); ++idx)
                print_byte(page.data_section[g_offset++]);
        }
        size_t number_of_elems_in_page = page.local_adj_list_size(0);
        for (size_t idx = 0; idx < number_of_elems_in_page; ++idx)
        {
            set_color(cdef_adj_page_id[idx % 2]);
            for (size_t _idx = 0; _idx < sizeof(PAGE_T::adj_page_id_t); ++_idx)
                print_byte(page.data_section[g_offset++]);
            set_color(cdef_adj_offset[idx % 2]);
            for (size_t _idx = 0; _idx < sizeof(PAGE_T::adj_offset_t); ++_idx)
                print_byte(page.data_section[g_offset++]);
            set_color(cdef_payload[idx % 2]);
            for (size_t _idx = 0; _idx < PAGE_T::adj_payload_size; ++_idx)
                print_byte(page.data_section[g_offset++]);
        }
    }
    else
    {
        // INVALID PAGE
        return;
    }

    size_t offset_slot_begin = page.DATA_SECTION_SIZE - number_of_slots * sizeof(typename PAGE_T::slot_t);
    set_color(cdef_unused);
    while (g_offset < offset_slot_begin)
        print_byte(page.data_section[g_offset++]);

    for (typename PAGE_T::offset_t slot_offset = 0;
         slot_offset < number_of_slots;
         ++slot_offset)
    {
        set_color(cdef_vertex_id[slot_offset % 2]);
        for (size_t _idx = 0; _idx < sizeof(PAGE_T::vertex_id_t); ++_idx)
            print_byte(page.data_section[g_offset++]);
        set_color(cdef_internal_offset[slot_offset % 2]);
        for (size_t _idx = 0; _idx < sizeof(PAGE_T::inter_idx_t); ++_idx)
            print_byte(page.data_section[g_offset++]);
    }

    set_color(cdef_footer_padding);
    for (size_t idx = 0; idx < sizeof(PAGE_T::footer_t::padding); ++idx)
        print_byte(page.data_section[g_offset++]);
    set_color(cdef_footer_flag);
    for (size_t idx = 0; idx < sizeof(PAGE_T::footer_t::flags); ++idx)
        print_byte(page.data_section[g_offset++]);
    set_color(cdef_footer_front);
    for (size_t idx = 0; idx < sizeof(PAGE_T::footer_t::front); ++idx)
        print_byte(page.data_section[g_offset++]);
    set_color(cdef_footer_rear);
    for (size_t idx = 0; idx < sizeof(PAGE_T::footer_t::rear); ++idx)
        print_byte(page.data_section[g_offset++]);

    set_color(cdef_origin);

#endif // !_WIN32
}

} // !namespace igraph

#endif // !_INFOGRAPH_TYPE_GENERIC_SLOTTED_PAGE_HELPER_ 