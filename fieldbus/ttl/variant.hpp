//  variant.hpp
//
//  Copyright (c) 2003 Eugene Gladyshev
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//

#ifndef __variant__hpp
#define __variant__hpp

#include "meta/typelist.hpp"

namespace ttl
{
namespace var
{
namespace impl
{
	struct variant_deleter
	{
		void *p_;
		variant_deleter( void *p ) : p_(p) {}
		
		template< typename T >
		void operator()() 
		{
			static_cast<data_holder<T>*>(p_)->~data_holder<T>();
		}
	};
	
	template< typename F, typename V >
	struct variant_binary_visitor
	{
		template<typename T1>
		struct visitor_functor
		{
			F& f_;
			T1 p1_;
			visitor_functor( F& f, T1 p1 ) : f_(f), p1_(p1) {}
			template < typename T2 >
			void operator()( T2 p2 ) { f_(p1_, p2); }
		};
		
		F& f_;
		V v_;

		variant_binary_visitor( F& f, V v ): f_(f), v_(v) {}

		template< typename T >
		void operator()( T p )
		{
			visitor_functor<T> f(f_,p);
			apply_visitor(f, v_);
		}
	};

	template< typename F >
	struct visitor_adapter_ref
	{
		F& f_;
		void *p_;
		visitor_adapter_ref( F& f, void *p ) : f_(f), p_(p) {}
		
		template< typename T >
		void operator()()
		{
			typedef data_holder<T> var_type;
			f_( static_cast<var_type*>(p_)->d_ );
		};
		
	};
};
	
			
	template< TTL_TPARAMS_DEF(TTL_MAX_TEMPLATE_PARAMS, empty_type) >
	struct variant
	{
		typedef meta::typelist<TTL_ARGS(TTL_MAX_TEMPLATE_PARAMS)> list;
		typedef variant<TTL_ARGS(TTL_MAX_TEMPLATE_PARAMS)> this_t;
		
	protected:
		struct copier
		{
			this_t& r_;
			const this_t& l_;

			copier( this_t& r, const this_t& l ) : r_(r), l_(l) {}

			template<typename T>
			void operator()()
			{
				typedef data_holder<T> var_type;
				r_ = static_cast<var_type*>(l_.get_data())->d_;
			};
		};
		
		
		template<int N> 
		struct storage
		{
			char buf_[N];
		};
		
	public:

		int which_;
		void* pnt_;
		storage< sizeof(data_holder<typename list::largest_type>) > stor_;

		variant() : which_(0), pnt_(0)
		{
		}

		template< typename T >
		variant( const T& l ) : which_(0), pnt_(0)
		{
			operator=(l);
		}
		variant( const this_t& l ) : which_(0), pnt_(0)
		{
			copier c(*this, l);
			meta::type_switch<list> ts;
			ts( l.which(), c );
		}
		
		virtual ~variant() { destroy(); }
		
		template< typename T >
		this_t& operator=( const T& l )
		{
			destroy();
			typedef meta::find<T, list> found;
			which_ = found::value;
			pnt_ = new(stor_.buf_) data_holder<typename found::type>(l);
			return *this;
		}
		
		this_t& operator=( const this_t& l )
		{
			if( this == &l ) return *this;
			copier c(*this, l);
			meta::type_switch<list> ts;
			ts( l.which(), c );
			return *this;
		}

		inline size_t get_types() const { return list::length; }
		inline int which() const { return which_; }
		inline bool is_singular() const { return pnt_ != 0; }
		inline void* get_data() const { return pnt_; }

	protected:
		void destroy()
		{
			if( !pnt_ ) return;
			void *p = pnt_;
			pnt_ = 0;
			impl::variant_deleter d(p);
			meta::type_switch<list> ts;
			ts(which_, d);
		}
	};
	
	
	template< typename T, typename V >
	T* get( V* v )
	{
		typedef meta::find<T, typename V::list> found;
		if( v->which() != found::value )
			throw std::runtime_error("invalid get");
		data_holder<typename found::type> *h = static_cast<data_holder<typename found::type>*>(v->get_data());
		return &h->d_;
	}
	
	template< typename T, typename V >
	T& get( V& v )
	{
		typedef meta::find<T, typename V::list> found;
		if( v.which() != found::value )
			throw std::runtime_error("invalid get");
		data_holder<typename found::type> *h = static_cast<data_holder<typename found::type>*>(v.get_data());
		return h->d_;
	}

	template< typename F, typename V >
	void apply_visitor( F& f, V& v )
	{
		if( !v.is_singular() ) return;
		meta::type_switch<typename V::list> ts;
		visitor_adapter_ref<F> va(f, v.get_data());
		ts(v.which(), va);
	}
	
	template< typename F, typename V1, typename V2 >
	void apply_visitor( F& f, V1& v1, V2& v2 )
	{
		variant_binary_visitor<F,V2&> bv(f,v2);
		apply_visitor(bv, v1);
	}
};
};

#endif //__variant__hpp
