/*--------------------------------------------------------------------*/
/* checkerFT.h                                                        */
/* Author: Daniel Park and John Hart                                  */
/*--------------------------------------------------------------------*/

#ifndef CHECKER_INCLUDED
#define CHECKER_INCLUDED

#include "NodeD.h"
#include "NodeF.h"

/* Returns TRUE if n represents a directory entry
   in a valid state, or FALSE otherwise. */
boolean CheckerFT_Dir_isValid(Node_D n);

/* Returns TRUE if n represents a file entry
   in a valid state, or FALSE otherwise. */
boolean CheckerFT_File_isValid(Node_F n);

/* Returns TRUE if the hierarchy is in a valid state or FALSE
   otherwise.  The data structure's validity is based on a boolean
   isInit indicating whether it has been initialized, a Node_D root
   representing the root of the hierarchy, and a size_t count
   representing the total number of directories in the hierarchy. */
boolean CheckerFT_isValid(boolean isInit, Node_D root, size_t count);

#endif
