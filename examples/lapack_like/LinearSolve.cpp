/*
   Copyright (c) 2009-2015, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License, 
   which can be found in the LICENSE file in the root directory, or at 
   http://opensource.org/licenses/BSD-2-Clause
*/
#include "El.hpp"
using namespace El;

int main( int argc, char* argv[] )
{
    Initialize( argc, argv );
    mpi::Comm comm = mpi::COMM_WORLD;
    const Int commRank = mpi::Rank( comm );
    const Int commSize = mpi::Size( comm );

    try 
    {
        const Int n = Input("--size","size of matrix",100);
        const Int numRhs = Input("--numRhs","# of right-hand sides",1); 
        const Int blocksize = Input("--blocksize","algorithmic blocksize",64);
        Int gridHeight = Input("--gridHeight","grid height",0);
        const bool details = Input("--details","print norm details?",false);
        const bool print = Input("--print","print matrices?",false);
        ProcessInput();
        PrintInputReport();

        // Set the algorithmic blocksize
        SetBlocksize( blocksize );

        // If the grid height wasn't specified, then we should attempt to build
        // a nearly-square process grid
        if( gridHeight == 0 )
            gridHeight = Grid::FindFactor( commSize );
        Grid grid( comm, gridHeight );

        // Set up random A and B, then make the copies X := B
        DistMatrix<> A(grid), B(grid), X(grid);
        for( Int test=0; test<3; ++test )
        {
            Uniform( A, n, n );
            Uniform( B, n, numRhs );
            X = B;
            if( print )
            {
                Print( A, "A" );
                Print( B, "B" );
            }

            // Perform the LU factorization and simultaneous solve
            if( commRank == 0 )
            {
                cout << "Starting linear solve...";
                cout.flush();
            }
            mpi::Barrier( comm );
            double startTime = mpi::Time();
            LinearSolve( A, X );
            mpi::Barrier( comm );
            double stopTime = mpi::Time();
            if( commRank == 0 )
                cout << stopTime-startTime << " seconds." << endl;

            // Form R := A X - B
            DistMatrix<> R( B );
            Gemm( NORMAL, NORMAL, 1., A, X, -1., R );

            // Compute the relevant Frobenius norms and a relative residual
            const double epsilon = lapack::MachineEpsilon();
            const double AFrobNorm = FrobeniusNorm( A );
            const double BFrobNorm = FrobeniusNorm( B );
            const double XFrobNorm = FrobeniusNorm( X );
            const double RFrobNorm = FrobeniusNorm( R );
            const double frobResidual = 
                RFrobNorm / (AFrobNorm*XFrobNorm*epsilon*n);
            if( commRank == 0 )
            {
                if( details )
                    cout << "||A||_F       = " << AFrobNorm << "\n"
                         << "||B||_F       = " << BFrobNorm << "\n"
                         << "||X||_F       = " << XFrobNorm << "\n"
                         << "||A X - B||_F = " << RFrobNorm << "\n";
                cout << "||A X - B||_F / (||A||_F ||X||_F epsilon n) = " 
                     << frobResidual << "\n" << endl;
            }

            // Compute the relevant infinity norms and a relative residual
            const double AInfNorm = InfinityNorm( A );
            const double BInfNorm = InfinityNorm( B );
            const double XInfNorm = InfinityNorm( X );
            const double RInfNorm = InfinityNorm( R );
            const double infResidual = RInfNorm / (AInfNorm*XInfNorm*epsilon*n);
            if( commRank == 0 )
            {
                if( details )
                    cout << "\n"
                         << "||A||_oo       = " << AInfNorm << "\n"
                         << "||B||_oo       = " << BInfNorm << "\n"
                         << "||X||_oo       = " << XInfNorm << "\n"
                         << "||A X - B||_oo = " << RInfNorm << "\n";
                cout << "||A X - B||_oo / (||A||_oo ||X||_oo epsilon n) = "
                     << infResidual << "\n" << endl;
            }

            // Compute the relevant one norms and a relative residual
            const double AOneNorm = OneNorm( A );
            const double BOneNorm = OneNorm( B );
            const double XOneNorm = OneNorm( X );
            const double ROneNorm = OneNorm( R );
            const double oneResidual = ROneNorm / (AOneNorm*XOneNorm*epsilon*n);
            if( commRank == 0 )
            {
                if( details )
                    cout << "\n"
                         << "||A||_1       = " << AOneNorm << "\n"
                         << "||B||_1       = " << BOneNorm << "\n"
                         << "||X||_1       = " << XOneNorm << "\n"
                         << "||A X - B||_1 = " << ROneNorm << "\n";
                cout << "||A X - B||_1 / (||A||_1 ||X||_1 epsilon n) = " 
                     << oneResidual << "\n" << endl;
            }
        }
    }
    catch( exception& e ) { ReportException(e); }

    Finalize();
    return 0;
}
