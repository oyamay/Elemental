/*
   Copyright 2009-2010 Jack Poulson

   This file is part of Elemental.

   Elemental is free software: you can redistribute it and/or modify it under
   the terms of the GNU Lesser General Public License as published by the
   Free Software Foundation; either version 3 of the License, or 
   (at your option) any later version.

   Elemental is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with Elemental. If not, see <http://www.gnu.org/licenses/>.
*/
#include "Elemental/BLASInternal.hpp"
using namespace std;
using namespace Elemental;

template<typename T>
void
Elemental::BLAS::Internal::SyrkLN
( const T alpha, const DistMatrix<T,MC,MR>& A,
  const T beta,        DistMatrix<T,MC,MR>& C )
{
#ifndef RELEASE
    PushCallStack("BLAS::Internal::SyrkLN");
    if( A.GetGrid() != C.GetGrid() )
        throw "A and C must be distributed over the same grid.";
    if( A.Height() != C.Height() || A.Height() != C.Width() )
    {
        ostringstream msg;
        msg << "Nonconformal SyrkLN:" << endl
            << "  A ~ " << A.Height() << " x " << A.Width() << endl
            << "  C ~ " << C.Height() << " x " << C.Width() <<  endl;
        const string s = msg.str();
        throw s.c_str();
    }
#endif
    const Grid& grid = A.GetGrid();

    // Matrix views
    DistMatrix<T,MC,MR> AL(grid), AR(grid),
                        A0(grid), A1(grid), A2(grid);

    // Temporary distributions
    DistMatrix<T,MC,  Star> A1_MC_Star(grid);
    DistMatrix<T,VR,  Star> A1_VR_Star(grid);
    DistMatrix<T,Star,MR  > A1Trans_Star_MR(grid);

    // Start the algorithm
    BLAS::Scal( beta, C );
    LockedPartitionRight( A, AL, AR );
    while( AR.Width() > 0 )
    {
        LockedRepartitionRight( AL, /**/ AR,
                                A0, /**/ A1, A2 );

        A1_MC_Star.AlignWith( C );
        A1_VR_Star.AlignWith( C );
        A1Trans_Star_MR.AlignWith( C );
        //--------------------------------------------------------------------//
        A1_VR_Star = A1_MC_Star = A1;
        A1Trans_Star_MR.TransposeFrom( A1_VR_Star );

        BLAS::Internal::TriangularRankK
        ( Lower, alpha, A1_MC_Star, A1Trans_Star_MR, (T)1, C );
        //--------------------------------------------------------------------//
        A1_MC_Star.FreeConstraints();
        A1_VR_Star.FreeConstraints();
        A1Trans_Star_MR.FreeConstraints();

        SlideLockedPartitionRight( AL,     /**/ AR,
                                   A0, A1, /**/ A2 );
    }
#ifndef RELEASE
    PopCallStack();
#endif
}

template void Elemental::BLAS::Internal::SyrkLN
( const float alpha, const DistMatrix<float,MC,MR>& A,
  const float beta,        DistMatrix<float,MC,MR>& C );

template void Elemental::BLAS::Internal::SyrkLN
( const double alpha, const DistMatrix<double,MC,MR>& A,
  const double beta,        DistMatrix<double,MC,MR>& C );

#ifndef WITHOUT_COMPLEX
template void Elemental::BLAS::Internal::SyrkLN
( const scomplex alpha, const DistMatrix<scomplex,MC,MR>& A,
  const scomplex beta,        DistMatrix<scomplex,MC,MR>& C );

template void Elemental::BLAS::Internal::SyrkLN
( const dcomplex alpha, const DistMatrix<dcomplex,MC,MR>& A,
  const dcomplex beta,        DistMatrix<dcomplex,MC,MR>& C );
#endif

