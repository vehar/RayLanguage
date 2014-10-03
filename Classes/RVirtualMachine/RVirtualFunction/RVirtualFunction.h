#ifndef __R_VIRTUAL_FUNCTION_H__
#define __R_VIRTUAL_FUNCTION_H__

#include "../../RayFoundation/RayFoundation.h"
#include "../../RayFoundation/RArray/RArray.h"
#include "../../RayFoundation/RByteOperations/RByteOperations.h"

class(RVirtualFunction)
    discipleOf(RByteArray)

    RCString   *name;
endOf(RVirtualFunction)

constructor (RVirtualFunction));
destructor  (RVirtualFunction);

printer(RVirtualFunction);


#endif /*__R_VIRTUAL_FUNCTION_H__*/