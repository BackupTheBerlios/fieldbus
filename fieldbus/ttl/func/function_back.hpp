//  function.hpp
//
//  Copyright (c) 2003 Eugene Gladyshev
//
//  Permission to copy, use, modify, sell and distribute this software
//  is granted provided this copyright notice appears in all copies.
//  This software is provided "as is" without express or implied
//  warranty, and with no claim as to its suitability for any purpose.
//

#ifndef __function__hpp
#define __function__hpp

#include "../config.hpp"
#include "../macro_params.hpp"

namespace ttl
{
namespace func
{
namespace impl
{
	template< typename R >
	struct functor_caller_base0
	{
		typedef R return_type;
		virtual return_type operator()() = 0;
	};
	template< typename F, typename R >
	struct functor_caller0 : functor_caller_base0<R>
	{
		typedef R return_type;
		F f_;
		functor_caller0( F f ) : f_(f) {}
		virtual return_type operator()() { return f_(); }
	};
	
	
	template< typename R, TTL_TPARAMS(1) >
	struct functor_caller_base1
	{
		typedef R return_type;
		virtual return_type operator()( TTL_FUNC_PARAMS(1,p) ) = 0;
	};
	template< typename F, typename R, TTL_TPARAMS(1) >
	struct functor_caller1< F, R (TTL_ARGS(1)) > : functor_caller_base1< R, TTL_ARGS(1) >
	{
		typedef R return_type;
		F f_;
		functor_caller1( F f ) : f_(f) {}
		virtual return_type operator()( TTL_FUNC_PARAMS(1,p) ) { return f_( TTL_ENUM_ITEMS(1,p) ); }
	};

/*
	template< typename R, TTL_TPARAMS_DEF(3, empty_type) >
	struct functor_caller_base
	{
		typedef R return_type;
		virtual return_type operator()( TTL_FUNC_PARAMS(3,p) ) = 0;
	};
	
	template< typename F, typename R, TTL_TPARAMS_DEF(3, empty_type) >
	struct functor_caller;
	*/
	
	///	specializations
	
	/*
	template< typename F, typename R >
	struct functor_caller<F, R, TTL_LIST_ITEMS(10,empty_type)>
		: functor_caller_base<R>
	{
		typedef R return_type;
		
		F f_;
		
		functor_caller( F f ) : f_(f) {}
		
		virtual return_type operator()( TTL_LIST_ITEMS(10,empty_type=empty_type()) )
		{
			return f_();
		}
	};
	template< typename F, typename R, TTL_TPARAMS(1) >
	struct functor_caller<F, R (TTL_ARGS(1)), TTL_LIST_ITEMS(9,empty_type)>
		: functor_caller_base<R, TTL_ARGS(1)>
	{
		typedef R return_type;
		
		F f_;
		
		functor_caller( F f ) : f_(f) {}
		
		virtual return_type operator()( TTL_FUNC_PARAMS(1,p), TTL_LIST_ITEMS(9,empty_type=empty_type()) )
		{
			return f_( TTL_ENUM_ITEMS(1,p) );
		}
	};
	
	//2 parameters
	template< typename F, typename R, TTL_TPARAMS(2) >
	struct functor_caller<F, R (TTL_ARGS(2)), TTL_LIST_ITEMS(8,empty_type)>
		: functor_caller_base<R, TTL_ARGS(2)>
	{
		typedef R return_type;
		
		F f_;
		
		functor_caller( F f ) : f_(f) {}
		
		virtual return_type operator()( TTL_FUNC_PARAMS(2,p), TTL_LIST_ITEMS(8,empty_type=empty_type()) )
		{
			return f_( TTL_ENUM_ITEMS(2,p) );
		}
	};
	
	#define TTL_BUILD_FUNCTOR_CALLER_START(params, rest) template< typename F, typename R, TTL_TPARAMS(params) >   \
	struct functor_caller<F, R (TTL_ARGS(params))>  \
		: functor_caller_base<R, TTL_ARGS(params)>  \
	{  \
		typedef R return_type;  \
		F f_;  \
		functor_caller( F f ) : f_(f) {}  \
		virtual return_type operator()( TTL_FUNC_PARAMS(params,p) )  \
		{  \
			return f_( TTL_ENUM_ITEMS(params,p) );  \
		}  \
	}; 
	
	#define TTL_BUILD_FUNCTOR_CALLER(params, rest) template< typename F, typename R, TTL_TPARAMS(params) >   \
	struct functor_caller<F, R (TTL_ARGS(params)), TTL_LIST_ITEMS(rest,empty_type)>  \
		: functor_caller_base<R, TTL_ARGS(params)>  \
	{  \
		typedef R return_type;  \
		F f_;  \
		functor_caller( F f ) : f_(f) {}  \
		virtual return_type operator()( TTL_FUNC_PARAMS(params,p), TTL_LIST_ITEMS(rest,empty_type=empty_type()) )  \
		{  \
			return f_( TTL_ENUM_ITEMS(params,p) );  \
		}  \
	}; 
	
	#define TTL_BUILD_FUNCTOR_CALLER_END(params, rest)  template< typename F, typename R > \
	struct functor_caller<F, R, TTL_LIST_ITEMS(rest,empty_type)> \
		: functor_caller_base<R> \
	{ \
		typedef R return_type; \
		F f_; \
		functor_caller( F f ) : f_(f) {} \
		virtual return_type operator()( TTL_LIST_ITEMS(rest,empty_type=empty_type()) ) \
		{ \
			return f_(); \
		} \
	};
	#define TTL_EMPTY()
//	TTL_REPEAT_BIDIR(3, TTL_BUILD_FUNCTOR_CALLER, TTL_BUILD_FUNCTOR_CALLER_END, 0)
	TTL_BUILD_FUNCTOR_CALLER_START(3,0)
	TTL_REPEAT_BIDIR(2, TTL_BUILD_FUNCTOR_CALLER, TTL_BUILD_FUNCTOR_CALLER, 1)
	TTL_BUILD_FUNCTOR_CALLER_END(0,3)
	
};
	*/

};
};

#endif //__function__hpp
