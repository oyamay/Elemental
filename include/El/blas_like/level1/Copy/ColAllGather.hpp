/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/
#ifndef EL_BLAS_COPY_COLALLGATHER_HPP
#define EL_BLAS_COPY_COLALLGATHER_HPP

namespace El {
namespace copy {

// (U,V) |-> (Collect(U),V)
template <Device D, typename T>
void ColAllGather_impl(const ElementalMatrix<T>& A, ElementalMatrix<T>& B)
{
    EL_DEBUG_CSE
    EL_DEBUG_ONLY(
      if (B.ColDist() != Collect(A.ColDist()) ||
          B.RowDist() != A.RowDist())
          LogicError("Incompatible distributions");
   )
    AssertSameGrids(A, B);

    const Int height = A.Height();
    const Int width = A.Width();
#ifdef EL_CACHE_WARNINGS
    if (height != 1 && A.Grid().Rank() == 0)
        Output
        ("The matrix redistribution [* ,V] <- [U,V] potentially causes a "
         "large amount of cache-thrashing. If possible, avoid it by "
         "performing the redistribution with a (conjugate)-transpose");
#endif
    B.AlignRowsAndResize(A.RowAlign(), height, width, false, false);

    // FIXME (trb): Actually sync properly after calls that use these!
    SyncInfo<D>
        syncInfoA = SyncInfoFromMatrix(static_cast<Matrix<T,D> const&>(A.LockedMatrix())),
        syncInfoB = SyncInfoFromMatrix(static_cast<Matrix<T,D> const&>(B.LockedMatrix()));

    auto syncHelper = MakeMultiSync(syncInfoB, syncInfoA);

    if (A.Participating())
    {
        const Int rowDiff = B.RowAlign()-A.RowAlign();
        if (rowDiff == 0)
        {
            if (A.ColStride() == 1)
            {
                Copy(A.LockedMatrix(), B.Matrix());
            }
            else if (height == 1)
            {
                if (A.ColRank() == A.ColAlign())
                    B.Matrix() = A.LockedMatrix();
                El::Broadcast(B, A.ColComm(), A.ColAlign());
            }
            else
            {
                const Int colStride = A.ColStride();
                const Int maxLocalHeight = MaxLength(height,colStride);
                const Int localWidth = A.LocalWidth();
                const Int portionSize = mpi::Pad(maxLocalHeight*localWidth);

                simple_buffer<T,D> buffer((colStride+1)*portionSize, syncInfoB);
                T* sendBuf = buffer.data();
                T* recvBuf = buffer.data() + portionSize;

                // Pack
                util::InterleaveMatrix(
                    A.LocalHeight(), localWidth,
                    A.LockedBuffer(), 1, A.LDim(),
                    sendBuf,          1, A.LocalHeight(), syncInfoB);

                // Communicate
                mpi::AllGather(
                    sendBuf, portionSize, recvBuf, portionSize, A.ColComm(),
                    syncInfoB);

                // Unpack
                util::ColStridedUnpack(
                    height, localWidth, A.ColAlign(), colStride,
                    recvBuf,    portionSize,
                    B.Buffer(), B.LDim(), syncInfoB);
            }
        }
        else
        {

#ifdef EL_UNALIGNED_WARNINGS
            if (A.Grid().Rank() == 0)
                cerr << "Unaligned [U,V] -> [* ,V]." << endl;
#endif
            const Int sendRowRank = Mod(A.RowRank()+rowDiff, A.RowStride());
            const Int recvRowRank = Mod(A.RowRank()-rowDiff, A.RowStride());

            if (height == 1)
            {
                const Int localWidthB = B.LocalWidth();
                simple_buffer<T,D> buffer(0, syncInfoB);
                T* bcastBuf;

                if (A.ColRank() == A.ColAlign())
                {
                    const Int localWidth = A.LocalWidth();
                    buffer.allocate(localWidth+localWidthB);
                    T* sendBuf = buffer.data();
                    bcastBuf   = buffer.data() + localWidth;

                    // Pack
                    util::DeviceStridedMemCopy(
                        sendBuf, 1, A.LockedBuffer(), A.LDim(), localWidth,
                        syncInfoB);

                    // Communicate
                    mpi::SendRecv(
                        sendBuf,  localWidth,  sendRowRank,
                        bcastBuf, localWidthB, recvRowRank, A.RowComm(),
                        syncInfoB);
                }
                else
                {
                    buffer.allocate(localWidthB);
                    bcastBuf = buffer.data();
                }

                // Communicate
                mpi::Broadcast(
                    bcastBuf, localWidthB, A.ColAlign(), A.ColComm(),
                    syncInfoB);

                util::DeviceStridedMemCopy(
                    B.Buffer(), B.LDim(), bcastBuf, 1, localWidthB,
                    syncInfoB);
            }
            else
            {
                const Int colStride = A.ColStride();
                const Int maxLocalHeight = MaxLength(height,colStride);
                const Int maxLocalWidth = MaxLength(width,A.RowStride());
                const Int portionSize =
                    mpi::Pad(maxLocalHeight*maxLocalWidth);

                simple_buffer<T,D> buffer((colStride+1)*portionSize, syncInfoB);
                T* firstBuf  = buffer.data();
                T* secondBuf = buffer.data() + portionSize;

                // Pack
                util::InterleaveMatrix(
                    A.LocalHeight(), A.LocalWidth(),
                    A.LockedBuffer(), 1, A.LDim(),
                    secondBuf,        1, A.LocalHeight(),
                    syncInfoB);

                // Realign
                mpi::SendRecv(
                    secondBuf, portionSize, sendRowRank,
                    firstBuf,  portionSize, recvRowRank, A.RowComm(),
                    syncInfoB);

                // AllGather the aligned data
                mpi::AllGather(
                    firstBuf,  portionSize,
                    secondBuf, portionSize, A.ColComm(), syncInfoB);

                // Unpack the contents of each member of the column team
                util::ColStridedUnpack(
                    height, B.LocalWidth(), A.ColAlign(), colStride,
                    secondBuf,  portionSize,
                    B.Buffer(), B.LDim(), syncInfoB);
            }
        }
    }
    // Consider a A[MD,STAR] -> B[STAR,STAR] redistribution, where only the
    // owning team of the MD distribution of A participates in the initial phase
    // and the second phase broadcasts over the cross communicator.
    if (A.Grid().InGrid() && A.CrossComm() != mpi::COMM_SELF)
        El::Broadcast(B, A.CrossComm(), A.Root());
}

template <typename T>
void ColAllGather
(const ElementalMatrix<T>& A, ElementalMatrix<T>& B)
{
    EL_DEBUG_CSE
    if (A.GetLocalDevice() != B.GetLocalDevice())
        LogicError(
            "ColAllGather: For now, A and B must be on same device.");

    switch (A.GetLocalDevice())
    {
    case Device::CPU:
        ColAllGather_impl<Device::CPU>(A,B);
        break;
#ifdef HYDROGEN_HAVE_CUDA
    case Device::GPU:
        ColAllGather_impl<Device::GPU>(A,B);
        break;
#endif // HYDROGEN_HAVE_CUDA
    default:
        LogicError("ColAllGather: Bad device.");
    }
}

template<typename T>
void ColAllGather
(const BlockMatrix<T>& A, BlockMatrix<T>& B)
{
    EL_DEBUG_CSE
    AssertSameGrids(A, B);

    EL_DEBUG_ONLY(
      if (A.RowDist() != B.RowDist() ||
          Collect(A.ColDist()) != B.ColDist())
          LogicError("Incompatible distributions");
   )
    const Int height = A.Height();
    const Int width = A.Width();
    const Int colCut = A.ColCut();
    const Int rowCut = A.RowCut();
    const Int blockHeight = A.BlockHeight();
    const Int blockWidth = A.BlockWidth();

    B.AlignAndResize
    (blockHeight, blockWidth, 0, A.RowAlign(), 0, rowCut,
      height, width, false, false);

    // TODO(poulson): Realign if the cuts are different
    if (A.BlockWidth() != B.BlockWidth() || A.RowCut() != B.RowCut())
    {
        EL_DEBUG_ONLY(
          Output("Performing expensive GeneralPurpose ColAllGather");
       )
        GeneralPurpose(A, B);
        return;
    }

    if (A.Participating())
    {
        const Int rowDiff = B.RowAlign() - A.RowAlign();
        const Int firstBlockHeight = blockHeight - colCut;
        if (rowDiff == 0)
        {
            if (A.ColStride() == 1)
            {
                Copy(A.LockedMatrix(), B.Matrix());
            }
            else if (height <= firstBlockHeight)
            {
                if (A.ColRank() == A.ColAlign())
                    B.Matrix() = A.LockedMatrix();
                El::Broadcast(B, A.ColComm(), A.ColAlign());
            }
            else
            {
                const Int colStride = A.ColStride();
                const Int localWidth = A.LocalWidth();
                const Int maxLocalHeight =
                  MaxBlockedLength(height,blockHeight,colCut,colStride);

                const Int portionSize = mpi::Pad(localWidth*maxLocalHeight);
                vector<T> buffer;
                FastResize(buffer, (colStride+1)*portionSize);
                T* sendBuf = &buffer[0];
                T* recvBuf = &buffer[portionSize];

                // Pack
                util::InterleaveMatrix(
                    A.LocalHeight(), localWidth,
                    A.LockedBuffer(), 1, A.LDim(),
                    sendBuf,          1, A.LocalHeight(),
                    SyncInfo<Device::CPU>{});

                // Communicate
                mpi::AllGather(
                    sendBuf, portionSize, recvBuf, portionSize, A.ColComm(),
                    SyncInfo<Device::CPU>{});

                // Unpack
                util::BlockedColStridedUnpack(
                    height, localWidth, A.ColAlign(), colStride,
                    A.BlockHeight(), A.ColCut(),
                    recvBuf, portionSize,
                    B.Buffer(), B.LDim());
            }
        }
        else
        {
#ifdef EL_UNALIGNED_WARNINGS
            if (A.Grid().Rank() == 0)
                Output("Unaligned ColAllGather");
#endif
            const Int sendRowRank = Mod(A.RowRank()+rowDiff, A.RowStride());
            const Int recvRowRank = Mod(A.RowRank()-rowDiff, A.RowStride());

            if (height <= firstBlockHeight)
            {
                if (A.ColRank() == A.ColAlign())
                    El::SendRecv
                    (A.LockedMatrix(), B.Matrix(),
                      A.RowComm(), sendRowRank, recvRowRank);
                El::Broadcast(B, A.ColComm(), A.ColAlign());
            }
            else
            {
                const Int colStride = A.ColStride();
                const Int localWidth = A.LocalWidth();
                const Int localHeightA = A.LocalHeight();
                const Int localWidthB = B.LocalWidth();
                const Int maxLocalWidth =
                  MaxBlockedLength(width,blockWidth,rowCut,A.RowStride());
                const Int maxLocalHeight =
                  MaxBlockedLength(height,blockHeight,colCut,colStride);

                const Int portionSize = mpi::Pad(maxLocalHeight*maxLocalWidth);
                vector<T> buffer;
                FastResize(buffer, (colStride+1)*portionSize);
                T* firstBuf = &buffer[0];
                T* secondBuf = &buffer[portionSize];

                // Pack
                util::InterleaveMatrix(
                    localHeightA, localWidth,
                    A.LockedBuffer(), 1, A.LDim(),
                    secondBuf,        1, localHeightA,
                    SyncInfo<Device::CPU>{});

                // Realign
                mpi::SendRecv(
                    secondBuf, portionSize, sendRowRank,
                    firstBuf,  portionSize, recvRowRank, A.RowComm(),
                    SyncInfo<Device::CPU>{});

                // Perform the column AllGather
                mpi::AllGather(
                    firstBuf,  portionSize,
                    secondBuf, portionSize, A.ColComm(),
                    SyncInfo<Device::CPU>{});

                // Unpack
                util::BlockedColStridedUnpack(
                    height, localWidthB, A.ColAlign(), colStride,
                    blockHeight, colCut, secondBuf, portionSize,
                    B.Buffer(), B.LDim());
            }
        }
    }
    // Consider a A[MD,STAR] -> B[STAR,STAR] redistribution, where only the
    // owning team of the MD distribution of A participates in the initial phase
    // and the second phase broadcasts over the cross communicator.
    if (A.Grid().InGrid() && A.CrossComm() != mpi::COMM_SELF)
        El::Broadcast(B, A.CrossComm(), A.Root());
}

} // namespace copy
} // namespace El

#endif // ifndef EL_BLAS_COPY_COLALLGATHER_HPP
