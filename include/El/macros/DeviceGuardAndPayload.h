/*
   Copyright (c) 2009-2016, Jack Poulson
   All rights reserved.

   This file is part of Elemental and is under the BSD 2-Clause License,
   which can be found in the LICENSE file in the root directory, or at
   http://opensource.org/licenses/BSD-2-Clause
*/

/* NOTE: This is a commonly-used hack that is only included in this file to
         help avoid redundancy. */
#define IF_GUARD_AND_PAYLOAD(CDIST,RDIST,WRAP,DEVICE) \
  if( GUARD(CDIST,RDIST,WRAP,DEVICE) ) { PAYLOAD(CDIST,RDIST,WRAP,DEVICE) }

#define ELSEIF_GUARD_AND_PAYLOAD(CDIST,RDIST,WRAP,DEVICE) \
  else IF_GUARD_AND_PAYLOAD(CDIST,RDIST,WRAP,DEVICE)

IF_GUARD_AND_PAYLOAD(CIRC,CIRC,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MC,  MR  ,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MC,  STAR,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MD,  STAR,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MR,  MC  ,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MR,  STAR,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,MC  ,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,MD  ,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,MR  ,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,STAR,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,VC  ,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,VR  ,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(VC,  STAR,ELEMENT,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(VR,  STAR,ELEMENT,Device::CPU)

#ifdef HYDROGEN_HAVE_CUDA
ELSEIF_GUARD_AND_PAYLOAD(CIRC,CIRC,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(MC,  MR  ,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(MC,  STAR,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(MD,  STAR,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(MR,  MC  ,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(MR,  STAR,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,MC  ,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,MD  ,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,MR  ,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,STAR,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,VC  ,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,VR  ,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(VC,  STAR,ELEMENT,Device::GPU)
ELSEIF_GUARD_AND_PAYLOAD(VR,  STAR,ELEMENT,Device::GPU)
#endif // HYDROGEN_HAVE_CUDA

ELSEIF_GUARD_AND_PAYLOAD(CIRC,CIRC,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MC,  MR  ,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MC,  STAR,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MD,  STAR,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MR,  MC  ,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(MR,  STAR,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,MC  ,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,MD  ,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,MR  ,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,STAR,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,VC  ,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(STAR,VR  ,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(VC,  STAR,BLOCK,Device::CPU)
ELSEIF_GUARD_AND_PAYLOAD(VR,  STAR,BLOCK,Device::CPU)
else
{
    LogicError("No (DIST,DIST,WRAP,DEVICE) match!");
}

#undef ELSEIF_GUARD_AND_PAYLOAD
#undef IF_GUARD_AND_PAYLOAD
#undef PAYLOAD
#undef GUARD
