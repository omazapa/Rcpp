// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; tab-width: 8 -*-
/* :tabSize=4:indentSize=4:noTabs=false:folding=explicit:collapseFolds=1: */
//
// wrap.h: Rcpp R/C++ interface class library -- wrap implementations
//
// Copyright (C) 2010	Dirk Eddelbuettel and Romain Francois
//
// This file is part of Rcpp.
//
// Rcpp is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Rcpp is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Rcpp.  If not, see <http://www.gnu.org/licenses/>.

#ifndef Rcpp_internal_wrap_h
#define Rcpp_internal_wrap_h

// this is a private header, included in RcppCommon.h
// don't include it directly

namespace Rcpp{

// pre-declaring wrap :
template <typename T> SEXP wrap(const T& object) ;

namespace internal{

// pre declaring
template <typename InputIterator> SEXP range_wrap(InputIterator first, InputIterator last) ;

// {{{ information about R vectors
// }}}

// {{{ range wrap 
// {{{ unnamed range wrap

template <typename FROM, typename TO> TO caster(FROM from){
	return static_cast<TO>(from) ;
}

/**
 * Range based primitive wrap implementation. used when 
 * - T is a primitive type, indicated by the r_type_traits
 * - T needs a static_cast to be of the type suitable to fit in the R vector
 *
 * This produces an unnamed vector of the appropriate type using the 
 * std::transform algorithm
 */
template <typename InputIterator, typename T>
SEXP primitive_range_wrap__impl( InputIterator first, InputIterator last, ::Rcpp::traits::true_type ){
	size_t size = std::distance( first, last ) ;
	const int RTYPE = ::Rcpp::traits::r_sexptype_traits<T>::rtype ;
	SEXP x = PROTECT( Rf_allocVector( RTYPE, size ) );
	std::transform( first, last, 
		r_vector_start< RTYPE, typename ::Rcpp::traits::storage_type<RTYPE>::type >(x), 
		caster< T, typename ::Rcpp::traits::storage_type<RTYPE>::type >
		) ; 
	UNPROTECT(1) ;
	return x ;
}

/**
 * Range based primitive wrap implementation. used when : 
 * - T is a primitive type
 * - T does not need a cast
 *
 * This produces an unnamed vector of the appropriate type using 
 * the std::copy algorithm
 */
template <typename InputIterator, typename T>
SEXP primitive_range_wrap__impl( InputIterator first, InputIterator last, ::Rcpp::traits::false_type ){
	size_t size = std::distance( first, last ) ;
	const int RTYPE = ::Rcpp::traits::r_sexptype_traits<T>::rtype ;
	SEXP x = PROTECT( Rf_allocVector( RTYPE, size ) );
	std::copy( first, last, r_vector_start<RTYPE, typename ::Rcpp::traits::storage_type<RTYPE>::type >(x) ) ; 
	UNPROTECT(1) ;
	return x ;
}


/**
 * Range based wrap implementation that deals with iterator over
 * primitive types (int, double, etc ...)
 * 
 * This produces an unnamed vector of the appropriate type
 */
template <typename InputIterator, typename T>
SEXP range_wrap_dispatch___impl( InputIterator first, InputIterator last, ::Rcpp::traits::r_type_primitive_tag){ 
	return primitive_range_wrap__impl<InputIterator,T>( first, last, typename ::Rcpp::traits::r_sexptype_needscast<T>() ) ;
} ;

/** 
 * range based wrap implementation that deals with iterators over 
 * some type U. each U object is itself wrapped
 * 
 * This produces an unnamed generic vector (list)
 */
template <typename InputIterator, typename T>
SEXP range_wrap_dispatch___impl( InputIterator first, InputIterator last, ::Rcpp::traits::r_type_generic_tag ){ 
	size_t size = std::distance( first, last ) ;
	SEXP x = PROTECT( Rf_allocVector( VECSXP, size ) );
	size_t i =0 ;
	while( i < size ){
		SET_VECTOR_ELT( x, i, ::Rcpp::wrap(*first) ) ;
		i++ ;
		++first ;
	}
	UNPROTECT(1) ;
	return x ;
} ;

/**
 * Range based wrap implementation for iterators over std::string
 * 
 * This produces an unnamed character vector
 */
template<typename InputIterator, typename T>
SEXP range_wrap_dispatch___impl( InputIterator first, InputIterator last, ::Rcpp::traits::r_type_string_tag ){
	size_t size = std::distance( first, last ) ;
	SEXP x = PROTECT( Rf_allocVector( STRSXP, size ) ) ;
	size_t i = 0 ;
	std::string buffer ;
	while( i < size ){
		buffer = *first ;
		SET_STRING_ELT( x, i, Rf_mkChar( buffer.c_str()) ) ;
		i++ ;
		++first ;
	}
	UNPROTECT(1) ;
	return x ;
}
// }}}

// {{{ named range wrap

/** 
 * range based wrap implementation that deals with iterators over
 * pair<const string,T> where T is a primitive type : int, double ...
 * 
 * This version is used when there is no need to cast T
 * 
 * This produces a named R vector of the appropriate type
 */
template <typename InputIterator, typename T>
SEXP range_wrap_dispatch___impl__cast( InputIterator first, InputIterator last, ::Rcpp::traits::false_type ){
	size_t size = std::distance( first, last ) ;
	const int RTYPE = ::Rcpp::traits::r_sexptype_traits<typename T::second_type>::rtype ;
	SEXP x = PROTECT( Rf_allocVector( RTYPE, size ) );
	SEXP names = PROTECT( Rf_allocVector( STRSXP, size ) ) ;
	typedef typename ::Rcpp::traits::storage_type<RTYPE>::type CTYPE ;
	CTYPE* start = r_vector_start<RTYPE,CTYPE>(x) ;
	size_t i =0;
	std::string buf ; 
	for( ; i<size; i++, ++first){
		start[i] = (*first).second ;
		buf = (*first).first ;
		SET_STRING_ELT( names, i, Rf_mkChar(buf.c_str()) ) ;
	}
	::Rf_setAttrib( x, R_NamesSymbol, names ) ;
	UNPROTECT(2) ; /* x, names */
	return x ;
} ;

/** 
 * range based wrap implementation that deals with iterators over
 * pair<const string,T> where T is a primitive type : int, double ...
 * 
 * This version is used when T needs to be cast to the associated R
 * type
 * 
 * This produces a named R vector of the appropriate type
 */
template <typename InputIterator, typename T>
SEXP range_wrap_dispatch___impl__cast( InputIterator first, InputIterator last, ::Rcpp::traits::true_type ){
	size_t size = std::distance( first, last ) ;
	const int RTYPE = ::Rcpp::traits::r_sexptype_traits<typename T::second_type>::rtype ;
	SEXP x = PROTECT( Rf_allocVector( RTYPE, size ) );
	SEXP names = PROTECT( Rf_allocVector( STRSXP, size ) ) ;
	typedef typename ::Rcpp::traits::storage_type<RTYPE>::type CTYPE ;
	CTYPE* start = r_vector_start<RTYPE,CTYPE>(x) ;
	size_t i =0;
	std::string buf ; 
	for( ; i<size; i++, ++first){
		start[i] = static_cast<CTYPE>( first->second );
		buf = first->first ;
		SET_STRING_ELT( names, i, Rf_mkChar(buf.c_str()) ) ;
	}
	::Rf_setAttrib( x, R_NamesSymbol, names ) ;
	UNPROTECT(2) ; /* x, names */
	return x ;
} ;


/** 
 * range based wrap implementation that deals with iterators over
 * pair<const string,T> where T is a primitive type : int, double ...
 * 
 * This dispatches further depending on whether the type needs 
 * a cast to fit into the associated R type
 * 
 * This produces a named R vector of the appropriate type
 */
template <typename InputIterator, typename T>
SEXP range_wrap_dispatch___impl( InputIterator first, InputIterator last, ::Rcpp::traits::r_type_pairstring_primitive_tag){ 
	return range_wrap_dispatch___impl__cast<InputIterator,T>( first, last, 
		typename ::Rcpp::traits::r_sexptype_needscast<typename T::second_type>() ) ;
} ;

/**
 * Range based wrap implementation that deals with iterators over
 * pair<const string, U> where U is wrappable. This is the kind of 
 * iterators that are produced by map<string,U>
 * 
 * This produces a named generic vector (named list). The first 
 * element of the list contains the result of a call to wrap on the 
 * object of type U, etc ...
 *
 * The names are taken from the keys
 */
template <typename InputIterator, typename T>
SEXP range_wrap_dispatch___impl( InputIterator first, InputIterator last, ::Rcpp::traits::r_type_pairstring_generic_tag ){ 
	size_t size = std::distance( first, last ) ;
	SEXP x = PROTECT( Rf_allocVector( VECSXP, size ) );
	SEXP names = PROTECT( Rf_allocVector( STRSXP, size ) ) ;
	size_t i =0 ;
	std::string buf ;
	SEXP element = R_NilValue ;
	while( i < size ){
		element = ::Rcpp::wrap( first->second ) ;
		buf = first->first ;
		SET_VECTOR_ELT( x, i, element ) ;
		SET_STRING_ELT( names, i, Rf_mkChar(buf.c_str()) ) ; 
		i++ ;
		++first ;
	}
	::Rf_setAttrib( x, R_NamesSymbol, names ) ;
	UNPROTECT(2) ; /* x, names */
	return x ;
} ;

/**
 * Range based wrap for iterators over std::pair<const std::string, std::string>
 *
 * This is mainly used for wrapping map<string,string> and friends 
 * which happens to produce iterators over pair<const string, string>
 *
 * This produces a character vector containing copies of the 
 * string iterated over. The names of the vector is set to the keys
 * of the pair
 */
template<typename InputIterator, typename T>
SEXP range_wrap_dispatch___impl( InputIterator first, InputIterator last, ::Rcpp::traits::r_type_pairstring_string_tag ){
	size_t size = std::distance( first, last ) ;
	SEXP x = PROTECT( Rf_allocVector( STRSXP, size ) ) ;
	SEXP names = PROTECT( Rf_allocVector( STRSXP, size ) ) ;
	size_t i = 0 ;
	std::string buffer ;
	while( i < size ){
		buffer = first->second ;
		SET_STRING_ELT( x, i, Rf_mkChar( buffer.c_str()) ) ;
		
		buffer = first->first ;
		SET_STRING_ELT( names, i, Rf_mkChar( buffer.c_str()) ) ;
		
		i++ ;
		++first ;
	}
	::Rf_setAttrib( x, R_NamesSymbol, names ) ;
	UNPROTECT(2) ; /* x, names */
	return x ;
}
// }}}

/**
 * Dispatcher for all range based wrap implementations
 * 
 * This uses the Rcpp::traits::r_type_traits to perform further dispatch
 */
template<typename InputIterator, typename T>
SEXP range_wrap_dispatch( InputIterator first, InputIterator last ){
	return range_wrap_dispatch___impl<InputIterator,T>( first, last, typename ::Rcpp::traits::r_type_traits<T>::r_category() ) ;
}

// we use the iterator trait to make the dispatch
/**
 * range based wrap. This uses the std::iterator_traits class
 * to perform further dispatch
 */
template <typename InputIterator>
SEXP range_wrap(InputIterator first, InputIterator last){
	return range_wrap_dispatch<InputIterator,typename std::iterator_traits<InputIterator>::value_type>( first, last ) ;
}
// }}}

// {{{ primitive wrap (wrapping a single primitive value)

/**
 * wraps a single primitive value when there is no need for a cast
 */
template <typename T>
SEXP primitive_wrap__impl__cast( const T& object, ::Rcpp::traits::false_type ){
	const int RTYPE = ::Rcpp::traits::r_sexptype_traits<T>::rtype ;
	SEXP x = PROTECT( Rf_allocVector( RTYPE, 1 ) );
	r_vector_start<RTYPE, typename ::Rcpp::traits::storage_type<RTYPE>::type >(x)[0] = object ;
	UNPROTECT(1);
	return x;
} ;

/**
 * wraps a single primitive value when a cast is needed
 */ 
template <typename T>
SEXP primitive_wrap__impl__cast( const T& object, ::Rcpp::traits::true_type ){
	const int RTYPE = ::Rcpp::traits::r_sexptype_traits<T>::rtype ;
	SEXP x = PROTECT( Rf_allocVector( RTYPE, 1 ) );
	r_vector_start<RTYPE, typename ::Rcpp::traits::storage_type<RTYPE>::type >(x)[0] = static_cast< typename ::Rcpp::traits::storage_type<RTYPE>::type >(object) ;
	UNPROTECT(1);
	return x;
} ;

/**
 * primitive wrap for 'easy' primitive types: int, double, Rbyte, Rcomplex
 *
 * This produces a vector of length 1 of the appropriate type
 */
template <typename T>
SEXP primitive_wrap__impl( const T& object, ::Rcpp::traits::r_type_primitive_tag ){
	return primitive_wrap__impl__cast( object, typename ::Rcpp::traits::r_sexptype_needscast<T>() ); 
} ;

/**
 * primitive wrap for types that can be converted implicitely to std::string
 * 
 * This produces a character vector of length 1 containing the std::string
 */
template <typename T>
SEXP primitive_wrap__impl( const T& object, ::Rcpp::traits::r_type_string_tag){
	SEXP x = PROTECT( ::Rf_allocVector( STRSXP, 1) ) ;
	std::string y = object ; /* give a chance to implicit conversion */
	SET_STRING_ELT( x, 0, Rf_mkChar(y.c_str()) ) ;
	UNPROTECT(1) ;
	return x; 
}

/**
 * called when T is a primitive type : int, bool, double, std::string, etc ...
 * This uses the Rcpp::traits::r_type_traits on the type T to perform
 * further dispatching and wrap the object into an vector of length 1
 * of the appropriate SEXP type 
 */
template <typename T>
SEXP primitive_wrap(const T& object){
	return primitive_wrap__impl( object, typename ::Rcpp::traits::r_type_traits<T>::r_category() ) ;
}
// }}}

// {{{ unknown
/**
 * Called when the type T is known to be implicitely convertible to 
 * SEXP. It uses the implicit conversion to SEXP to wrap the object
 * into a SEXP
 */
template <typename T>
SEXP wrap_dispatch_unknown( const T& object, ::Rcpp::traits::true_type ){
	// here we know (or assume) that T is convertible to SEXP
	SEXP x = object ;
	return x ;
}

/**
 * This is the worst case : 
 * - not a primitive
 * - not implicitely convertible tp SEXP
 * - not iterable
 *
 * so we just give up and attempt to use static_assert to generate 
 * a compile time message if it is available, otherwise we use 
 * implicit conversion to SEXP to bomb the compiler, which will give
 * quite a cryptic message
 */
template <typename T>
SEXP wrap_dispatch_unknown_iterable(const T& object, ::Rcpp::traits::false_type){
	// here we know that T is not convertible to SEXP
#ifdef HAS_CXX0X
	static_assert( !sizeof(T), "cannot convert type to SEXP" ) ;
#else
	// leave the cryptic message
	SEXP x = object ; 
	return x ;
#endif
	return R_NilValue ; // -Wall
}

/**
 * Here we know for sure that type T has a T::iterator typedef
 * so we hope for the best and call the range based wrap with begin
 * and end
 *
 * This works fine for all stl containers and classes T that have : 
 * - T::iterator
 * - T::iterator begin()
 * - T::iterator end()
 *
 * If someone knows a better way, please advise
 */
template <typename T>
SEXP wrap_dispatch_unknown_iterable(const T& object, ::Rcpp::traits::true_type){
	return range_wrap( object.begin(), object.end() ) ;
}

template <typename T, typename elem_type>
SEXP wrap_dispatch_importer__impl__prim( const T& object, ::Rcpp::traits::false_type ){
	int size = object.size() ;
	const int RTYPE = ::Rcpp::traits::r_sexptype_traits<elem_type>::rtype ;
	SEXP x = PROTECT( Rf_allocVector( RTYPE, size ) );
	typedef typename ::Rcpp::traits::storage_type<RTYPE>::type CTYPE ;
	CTYPE* start = r_vector_start<RTYPE,CTYPE>(x) ;
	for( int i=0; i<size; i++){
		start[i] = object.get(i) ;
	}
	UNPROTECT(1) ;
	return x ;

}

template <typename T, typename elem_type>
SEXP wrap_dispatch_importer__impl__prim( const T& object, ::Rcpp::traits::true_type ){
	int size = object.size() ;
	const int RTYPE = ::Rcpp::traits::r_sexptype_traits<elem_type>::rtype ;
	SEXP x = PROTECT( Rf_allocVector( RTYPE, size ) );
	typedef typename ::Rcpp::traits::storage_type<RTYPE>::type CTYPE ;
	CTYPE* start = r_vector_start<RTYPE,CTYPE>(x) ;
	for( int i=0; i<size; i++){
		start[i] = caster<elem_type,CTYPE>( object.get(i) );
	}
	UNPROTECT(1) ;
	return x ;
}

template <typename T, typename elem_type>
SEXP wrap_dispatch_importer__impl( const T& object, ::Rcpp::traits::wrap_type_primitive_tag ){
	return wrap_dispatch_importer__impl__prim<T,elem_type>( object, 
		typename ::Rcpp::traits::r_sexptype_needscast<elem_type>() ) ;
}

template <typename T, typename elem_type>
SEXP wrap_dispatch_importer( const T& object ){
	return wrap_dispatch_importer__impl<T,elem_type>( object, 
		typename ::Rcpp::traits::r_type_traits<elem_type>::r_category() 
		 ) ;
}

/** 
 * Called when no implicit conversion to SEXP is possible and this is 
 * not tagged as a primitive type, checks whether the type is 
 * iterable
 */
template <typename T>
SEXP wrap_dispatch_unknown( const T& object, ::Rcpp::traits::false_type){
	return wrap_dispatch_unknown_iterable( object, typename ::Rcpp::traits::has_iterator<T>::type() ) ;
}
// }}}

// {{{ wrap dispatch
/**
 * wrapping a __single__ primitive type : int, double, std::string, size_t, 
 * Rbyte, Rcomplex
 */
template <typename T> SEXP wrap_dispatch( const T& object, ::Rcpp::traits::wrap_type_primitive_tag ){
	return primitive_wrap( object ) ;
}

/**
 * called when T is wrap_type_unknown_tag and is not an Importer class
 * The next step is to try implicit conversion to SEXP
 */
template <typename T> SEXP wrap_dispatch_unknown_importable( const T& object, ::Rcpp::traits::false_type){
	return wrap_dispatch_unknown( object, typename is_convertible<T,SEXP>::type() ) ;
}

/**
 * called when T is an Importer
 */
template <typename T> SEXP wrap_dispatch_unknown_importable( const T& object, ::Rcpp::traits::true_type){
	return wrap_dispatch_importer<T,typename T::r_import_type>( object ) ;
}
 
 
/** 
 * This is called by wrap when the wrap_type_traits is wrap_type_unknown_tag
 * 
 * This tries to identify if the object conforms to the Importer class
 */
template <typename T> SEXP wrap_dispatch( const T& object, ::Rcpp::traits::wrap_type_unknown_tag ){
	return wrap_dispatch_unknown_importable( object, typename ::Rcpp::traits::is_importer<T>::type() ) ;
}
	// }}}

} // internal

/**
 * wraps an object of type T in a SEXP
 *
 * This method depends on the Rcpp::traits::wrap_type_traits trait 
 * class to dispatch to the appropriate internal implementation 
 * method
 * 
 * If your type has a begin and end method returning stl-like iterator
 * you should specialize the wrap_type_traits template so that it 
 * defines wrap_category to be ::Rcpp::traits::wrap_type_stl_container_tag
 */
template <typename T> SEXP wrap(const T& object){
	return internal::wrap_dispatch( object, typename ::Rcpp::traits::wrap_type_traits<T>::wrap_category() ) ;
}

// special case - FIXME : this is not template specializations of wrap<>
inline SEXP wrap(const char* const v ){ return Rf_mkString(v) ; } ;

/**
 * Range based version of wrap
 */
template <typename InputIterator>
SEXP wrap(InputIterator first, InputIterator last){ return internal::range_wrap( first, last ) ; }


} // Rcpp

#endif
