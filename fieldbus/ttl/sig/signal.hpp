//  signal.hpp
//
//  Copyright (c) 2003 Eugene Gladyshev
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//

#ifndef __ttl_signal__hpp
#define __ttl_signal__hpp

#include <list>
#include "../func/function.hpp"

namespace ttl
{
namespace sig
{
struct exception : ttl::exception
{
	exception() : ttl::exception("signal error") {}
};

/////////////////////////////////////////////////////////////
namespace impl
{
	struct connection_holder_base
	{
		typedef ttl::mem::memtraits::allocator<connection_holder_base> allocator;
		bool active_;
		int ref_;
		connection_holder_base() : active_(false), ref_(0) {}
		virtual ~connection_holder_base() {}
		virtual void disconnect() = 0;
		inline bool is_active() const { return active_; }
	};
}; //impl

/////////////////////////////////////////////////////////////////////
	struct connection
	{
		typedef impl::connection_holder_base holder;
		connection() : c_(0) {}
		connection( holder* c ) : c_(c) { ++c_->ref_; }
		connection( const connection& c ) : c_(c.c_) { ++c_->ref_; }
		~connection() { release(); }
		inline void disconnect()
		{
			if(c_) c_->disconnect();
		}
		inline bool is_active() const { return c_ && c_->is_active(); }
		connection& operator=( const connection& c )
		{
			if( &c == this ) return *this;
			release();
			c_ = c.c_;
			++c_->ref_;
			return *this;
		}
		
	private:
		holder* c_;
		
		void release()
		{
			if( c_ && (!--c_->ref_) ) 
			{
				ttl::mem::destroy<holder::allocator>(c_);
				c_ = 0;
			}
		}
	};
	
namespace impl
{
	template< typename F >
	struct signal_base
	{
		struct connection_holder;
		typedef std::list<connection_holder*> list;

		struct connection_holder : impl::connection_holder_base
		{
			list& l_;
			typename list::iterator it_;
			F f_;
			
			connection_holder( list& l, const F& f ) : l_(l), f_(f) {}
			connection connect()
			{
				if( is_active() ) throw ttl::sig::exception();
				it_ = l_.insert( l_.end(), this );
				active_ = true;
				++ref_;
				return connection(this);
			}
			
			virtual void disconnect()
			{
				if( !is_active() ) return;
				l_.erase(it_);
				active_ = false;
				--ref_;
			}
		};
		typedef typename ttl::mem::memtraits::allocator<connection_holder> allocator;
		
		signal_base() {}
		virtual ~signal_base() { clear(); }
		
		connection connect( const F& f )
		{
			connection_holder t( con_, f );
			connection_holder *c = ttl::mem::create<allocator>( t );
			if( !c ) throw ttl::sig::exception();
			return c->connect();
		}
	protected:
		list con_;
		void clear()
		{
			typename list::const_iterator it;
			for( it = con_.begin(); it != con_.end(); it = con_.begin() )
			{
				connection_holder *c = *it;
				c->disconnect();
				if( !c->ref_ )
				{
					ttl::mem::destroy<allocator>(c);
				}
			}
		}
	 private:
		//protect from copy
		signal_base( const signal_base& );
		const signal_base& operator=( const signal_base& );
	};
};  //impl


	template < typename R, TTL_TPARAMS_DEF(TTL_MAX_FUNCTION_PARAMS, empty_type) >
	struct signal : impl::signal_base< func::function<R,TTL_ARGS(TTL_MAX_FUNCTION_PARAMS) > >
	{
		typedef func::function< R,TTL_ARGS(TTL_MAX_FUNCTION_PARAMS) > slot_type;
		typedef impl::signal_base<slot_type> base_t;
		typedef typename base_t::list list;
		
		void operator()()
		{
			typename list::const_iterator it;
			for( it = con_.begin(); it != con_.end(); ++it )
			{
				(*it)->f_();
			}
		}
	};

#define TTL_SIG_BUILD_SIGNAL(n) \
		typedef impl::signal_base<slot_type> base_t; \
		typedef typename base_t::list list; \
		void operator()(TTL_FUNC_PARAMS(n,p)) \
		{ \
			typename list::const_iterator it; \
			for( it = con_.begin(); it != con_.end(); ++it ) \
			{ \
				(*it)->f_(TTL_ENUM_ITEMS(n,p)); \
			} \
		}
	
	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(1)), T2, T3, T4, T5, T6, T7, T8, T9, T10 > : 
		impl::signal_base< func::function<R (TTL_ARGS(1)), T2, T3, T4, T5, T6, T7, T8, T9, T10> >
	{
		typedef func::function<R (TTL_ARGS(1)), T2, T3, T4, T5, T6, T7, T8, T9, T10> slot_type;
		TTL_SIG_BUILD_SIGNAL(1)
	};
	
	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(2)), T3, T4, T5, T6, T7, T8, T9, T10 > : 
		impl::signal_base< func::function<R (TTL_ARGS(2)), T3, T4, T5, T6, T7, T8, T9, T10> >
	{
		typedef func::function<R (TTL_ARGS(2)), T3, T4, T5, T6, T7, T8, T9, T10> slot_type;
		TTL_SIG_BUILD_SIGNAL(2)
	};
	
	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(3)), T4, T5, T6, T7, T8, T9, T10 > : 
		impl::signal_base< func::function<R (TTL_ARGS(3)), T4, T5, T6, T7, T8, T9, T10> >
	{
		typedef func::function<R (TTL_ARGS(3)), T4, T5, T6, T7, T8, T9, T10> slot_type;
		TTL_SIG_BUILD_SIGNAL(3)
	};
	
	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(4)), T5, T6, T7, T8, T9, T10 > : 
		impl::signal_base< func::function<R (TTL_ARGS(4)), T5, T6, T7, T8, T9, T10> >
	{
		typedef func::function<R (TTL_ARGS(4)), T5, T6, T7, T8, T9, T10> slot_type;
		TTL_SIG_BUILD_SIGNAL(4)
	};
	
	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(5)), T6, T7, T8, T9, T10 > : 
		impl::signal_base< func::function<R (TTL_ARGS(5)), T6, T7, T8, T9, T10> >
	{
		typedef func::function<R (TTL_ARGS(5)), T6, T7, T8, T9, T10> slot_type;
		TTL_SIG_BUILD_SIGNAL(5)
	};
	
	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(6)), T7, T8, T9, T10 > : 
		impl::signal_base< func::function<R (TTL_ARGS(6)), T7, T8, T9, T10> >
	{
		typedef func::function<R (TTL_ARGS(6)), T7, T8, T9, T10> slot_type;
		TTL_SIG_BUILD_SIGNAL(6)
	};
	
	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(7)), T8, T9, T10 > : 
		impl::signal_base< func::function<R (TTL_ARGS(7)), T8, T9, T10> >
	{
		typedef func::function<R (TTL_ARGS(7)), T8, T9, T10> slot_type;
		TTL_SIG_BUILD_SIGNAL(7)
	};

	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(8)), T9, T10 > : 
		impl::signal_base< func::function<R (TTL_ARGS(8)), T9, T10> >
	{
		typedef func::function<R (TTL_ARGS(8)), T9, T10> slot_type;
		TTL_SIG_BUILD_SIGNAL(8)
	};
	
	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(9)), T10 > : 
		impl::signal_base< func::function<R (TTL_ARGS(9)), T10> >
	{
		typedef func::function<R (TTL_ARGS(9)), T10> slot_type;
		TTL_SIG_BUILD_SIGNAL(9)
	};

	template < typename R, TTL_TPARAMS(TTL_MAX_FUNCTION_PARAMS) >
	struct signal<R (TTL_ARGS(10)) > : 
		impl::signal_base< func::function<R (TTL_ARGS(10)) > >
	{
		typedef func::function< R (TTL_ARGS(10)) > slot_type;
		TTL_SIG_BUILD_SIGNAL(10)
	};
}; //sig
}; //ttl

#endif //__ttl_signal__hpp
