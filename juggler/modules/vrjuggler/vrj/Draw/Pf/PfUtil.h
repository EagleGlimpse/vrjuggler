#ifndef _VJ_PF_UTIL_H_
#define _VJ_PF_UTIL_H_

/**************************************************
* Performer helper routines                       *
***************************************************/

#include <vjConfig.h>

#include <Performer/pr/pfLinMath.h>
#include <Math/vjMatrix.h>


//: Convert Performer matrix to Juggler matrix
vjMatrix vjGetVJMatrix(pfMatrix& perfMat);

//: Convert Juggler Matrix to Pf Matrix
pfMatrix vjGetPfMatrix(vjMatrix& mat);


#endif
