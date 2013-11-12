// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
//
// Vector.h: Rcpp R/C++ interface class library -- vectors
//
// Copyright (C) 2010 - 2013 Dirk Eddelbuettel and Romain Francois
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

#ifndef Rcpp__vector__no_init_h
#define Rcpp__vector__no_init_h
  
namespace Rcpp{
    
class no_init {
public:
    no_init(int size_): size(size_){}
    
    inline int get() const { 
        return size; 
    }
    
    template <int RTYPE, template <class> class StoragePolicy >
    operator Vector<RTYPE, StoragePolicy>(){ 
        return Rf_allocVector(RTYPE, size) ; 
    }
    
private:
    int size ;
} ;
  
}
#endif
