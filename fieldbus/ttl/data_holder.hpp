//  data_holder.hpp
//
//  Copyright (c) 2003 Eugene Gladyshev
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//

#ifndef __ttl_impl_data_holder__hpp
#define __ttl_impl_data_holder__hpp

namespace ttl
{
namespace impl
{
	struct data_holder_base {};
	
	template< typename T >
	struct data_holder : data_holder_base
	{
		typedef T type;
		typedef T& reference;
		typedef const T& const_reference;
		typedef const T& call_param;
		typedef T* pointer;
		
		type d_;
		
		data_holder( call_param l ) : d_(l) 
		{
		}
	};

	template< typename T >
	struct data_holder<T&> : data_holder_base
	{
		typedef T& type;
		typedef T& reference;
		typedef const T& const_reference;
		typedef reference call_param;
		typedef T* pointer;
		
		type d_;
		
		data_holder( reference l ) : d_(l)
		{
		}
	};
	
	template< typename T >
	struct data_holder<const T&> : data_holder_base
	{
		typedef const T& type;
		typedef T& reference;
		typedef const T& const_reference;
		typedef const_reference call_param;
		typedef const T* pointer;
		
		type d_;
		
		data_holder( call_param l ) : d_(l)
		{
		}
	};
	
	template< typename T >
	struct data_holder<T*> : data_holder_base
	{
		typedef T* type;
		typedef type& reference;
		typedef const type& const_reference;
		typedef type call_param;
		typedef T* pointer;
		
		type d_;
		
		data_holder( call_param l ) : d_(l) {}
	};
	
	template< typename T >
	struct data_holder<const T*> : data_holder_base
	{
		typedef const T* type;
		typedef type& reference;
		typedef const type& const_reference;
		typedef type call_param;
		typedef const T* pointer;
		
		type d_;
		
		data_holder( call_param l ) : d_(l) {}
	};
};
};

#endif //__data_holder__hpp
