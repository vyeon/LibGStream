/** -------------------------------------------------------------------
*	@project	LibGStream
*	@location	gstream/cuda/datatype
*	@file		device_slotted_page.h
*	@brief		Generic slotted page for CUDA device code
*	@author		Seyeon Oh (vee@dgist.ac.kr)
*	@version	1.0, 29/3/2017
* ----------------------------------------------------------------- */

#ifndef _GSTREAM_CUDA_DATATYPE_DEVICE_SLOTTED_PAGE_H_
#define _GSTREAM_CUDA_DATATYPE_DEVICE_SLOTTED_PAGE_H_

#include <gstream/datatype/slotted_page.h>

/* CUDA headers */
#include <host_defines.h>

namespace gstream {

namespace cuda {

#pragma pack (push, 1)
template <
    typename __vertex_id_t,
    typename __page_id_t,
    typename __record_offset_t,
    typename __slot_offset_t,
    typename __record_size_t,
    size_t   __page_size,
    typename __edge_payload_t = void,
    typename __vertex_payload_t = void,
    typename __offset_t = _slotted_page::default_offset_t
>
class device_slotted_page {
	/* Constratint 1. The edge-payload type must be a void type or a Plain old data (POD) type */
	static_assert((std::is_void<__edge_payload_t>::value || std::is_pod<__edge_payload_t>::value),
		"Generic Slotted Page: Constraint 1. The edge-payload type must be a Plain old data (POD) type");
	/* Constratint 2. The vertex-payload type must be a void type or a Plain old data (POD) type */
	static_assert((std::is_void<__vertex_payload_t>::value || std::is_pod<__vertex_payload_t>::value),
		"Generic Slotted Page: Constraint 2. The vertex-payload type must be a Plain old data (POD) type"); 

	/* Typedefs and Constant value definitions */
public:
	using type = device_slotted_page<__GSTREAM_SLOTTED_PAGE_TEMPLATE_ARGS>;
	using shared_ptr = std::shared_ptr<type>;
	using unique_ptr = std::unique_ptr<type>;
	using weak_ptr = std::weak_ptr<type>;
	__GSTREAM_SLOTTED_PAGE_TEMPLATE_TYPEDEFS;
	__GSTREAM_SLOTTED_PAGE_TEMPLATE_CONSTDEFS;

    //! Disallow Constructors & Destructor
    device_slotted_page() = delete;
    device_slotted_page(const type& other) = delete;
    ~device_slotted_page() = delete;

	/* Member functions */ 
    
	// Operators
	__device__ inline type& operator=(const type& other)
	{
		memmove(this, &other, PageSize);
		return *this;
	}

	inline uint8_t& operator[](offset_t offset)
	{
		return data_section[offset];
	}
	inline bool operator==(const type& other)
	{
		return memcmp(this, &other, sizeof(PageSize)) == 0;
	}

	// Utilites
	inline offset_t number_of_slots() const
	{
		return static_cast<offset_t>((DataSectionSize - this->footer.rear) / sizeof(slot_t));
	}
	inline slot_t& slot(const offset_t offset)
	{
		return *reinterpret_cast<slot_t*>(&this->data_section[DataSectionSize - (sizeof(slot_t) * (offset + 1))]);
	}
	inline record_size_t& record_size(const slot_t& slot)
	{
		return *reinterpret_cast<record_size_t*>(&data_section[slot.record_offset]);
	}
	inline record_size_t& record_size(const offset_t slot_offset)
	{
		return this->record_size(slot(slot_offset));
	}
	inline adj_list_elem_t* list(const slot_t& slot)
	{
		return reinterpret_cast<adj_list_elem_t*>(&data_section[slot.record_offset + sizeof(record_size_t)]);
	}
	inline adj_list_elem_t* list(const offset_t slot_offset)
	{
		return reinterpret_cast<adj_list_elem_t*>(&data_section[slot(slot_offset).record_offset + sizeof(record_size_t)]);
	}
	inline adj_list_elem_t* list_ext(const slot_t& slot)
	{
		return reinterpret_cast<adj_list_elem_t*>(&data_section[slot.record_offset]);
	}
	inline adj_list_elem_t* list_ext(const offset_t slot_offset)
	{
		return reinterpret_cast<adj_list_elem_t*>(&data_section[slot(slot_offset).record_offset]);
	}
	inline page_flag_t& flags()
	{
		return footer.flags;
	}

	inline bool is_lp() const
	{
		return 0 != (footer.flags & (slotted_page_flag::LP_HEAD | slotted_page_flag::LP_EXTENDED));
	}
	inline bool is_lp_head() const
	{
		return 0 != (footer.flags & slotted_page_flag::LP_HEAD);
	}
	inline bool is_lp_extended() const
	{
		return 0 != (footer.flags & slotted_page_flag::LP_EXTENDED);
	}
	inline bool is_sp() const
	{
		return 0 != (footer.flags & slotted_page_flag::SP);
	}
	inline bool is_empty() const
	{
		return (footer.front == 0 && footer.rear == DataSectionSize);
	}

	/* Member variables */
public:
	uint8_t  data_section[DataSectionSize];
	footer_t footer{ 0, 0, 0, DataSectionSize };
};
#pragma pack(pop)

} // !namespace cuda

} // !namespace gstream

#endif // !_GSTREAM_CUDA_DATATYPE_DEVICE_SLOTTED_PAGE_H_