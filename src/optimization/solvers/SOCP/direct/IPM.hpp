/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"

namespace El {
namespace socp {
namespace direct {

/*
template<typename Real>
void IPF
( const Matrix<Real>& A, 
  const Matrix<Real>& b, 
  const Matrix<Real>& c,
  const Matrix<Int>& orders,
  const Matrix<Int>& firstInds,
  const Matrix<Int>& labels,
        Matrix<Real>& x, 
        Matrix<Real>& y,
        Matrix<Real>& z, 
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>() );
template<typename Real>
void IPF
( const AbstractDistMatrix<Real>& A, 
  const AbstractDistMatrix<Real>& b, 
  const AbstractDistMatrix<Real>& c,
  const AbstractDistMatrix<Int>& orders,
  const AbstractDistMatrix<Int>& firstInds,
  const AbstractDistMatrix<Int>& labels,
        AbstractDistMatrix<Real>& x, 
        AbstractDistMatrix<Real>& y,
        AbstractDistMatrix<Real>& z, 
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>() );
template<typename Real>
void IPF
( const SparseMatrix<Real>& A, 
  const Matrix<Real>& b, 
  const Matrix<Real>& c,
  const Matrix<Int>& orders,
  const Matrix<Int>& firstInds,
  const Matrix<Int>& labels,
        Matrix<Real>& x, 
        Matrix<Real>& y,
        Matrix<Real>& z,
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>() );
template<typename Real>
void IPF
( const DistSparseMatrix<Real>& A, 
  const DistMultiVec<Real>& b, 
  const DistMultiVec<Real>& c,
  const DistMultiVec<Int>& orders,
  const DistMultiVec<Int>& firstInds,
  const DistMultiVec<Int>& labels,
        DistMultiVec<Real>& x, 
        DistMultiVec<Real>& y,
        DistMultiVec<Real>& z, 
  const IPFCtrl<Real>& ctrl=IPFCtrl<Real>() );
*/

template<typename Real>
void Mehrotra
( const Matrix<Real>& A,
  const Matrix<Real>& b,
  const Matrix<Real>& c,
  const Matrix<Int>& orders,
  const Matrix<Int>& firstInds,
  const Matrix<Int>& labels,
        Matrix<Real>& x,
        Matrix<Real>& y,
        Matrix<Real>& z,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>() );
template<typename Real>
void Mehrotra
( const AbstractDistMatrix<Real>& A,
  const AbstractDistMatrix<Real>& b,
  const AbstractDistMatrix<Real>& c,
  const AbstractDistMatrix<Int>& orders,
  const AbstractDistMatrix<Int>& firstInds,
  const AbstractDistMatrix<Int>& labels,
        AbstractDistMatrix<Real>& x,
        AbstractDistMatrix<Real>& y,
        AbstractDistMatrix<Real>& z,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>() );
template<typename Real>
void Mehrotra
( const SparseMatrix<Real>& A, 
  const Matrix<Real>& b, 
  const Matrix<Real>& c,
  const Matrix<Int>& orders,
  const Matrix<Int>& firstInds,
  const Matrix<Int>& labels,
        Matrix<Real>& x, 
        Matrix<Real>& y,
        Matrix<Real>& z, 
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>() );
template<typename Real>
void Mehrotra
( const DistSparseMatrix<Real>& A,
  const DistMultiVec<Real>& b,
  const DistMultiVec<Real>& c,
  const DistMultiVec<Int>& orders,
  const DistMultiVec<Int>& firstInds,
  const DistMultiVec<Int>& labels,
        DistMultiVec<Real>& x,
        DistMultiVec<Real>& y,
        DistMultiVec<Real>& z,
  const MehrotraCtrl<Real>& ctrl=MehrotraCtrl<Real>() );

} // namespace direct
} // namespace socp
} // namespace El
