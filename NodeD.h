/*--------------------------------------------------------------------*/
/* NodeD.h                                                          */
/* Author: John Hart and Daniel Park                                  */
/*--------------------------------------------------------------------*/

#ifndef DIRNODE_INCLUDED
#define DIRNODE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "nodes.h"

/* Destroys the entire hierarchy of nodes rooted at n, including n
itself. Returns the number of nodes destroyed. */

size_t NodeD_destroy(Node_D n);

/* Compares node1 and node2 based on their paths.
   Returns <0, 0, or >0 if node1 is less than,
   equal to, or greater than node2, respectively. */

int NodeD_compare(Node_D node1, Node_D node2);

/* Returns n's path. Return NULL if n is null or the operation fails */
const char* NodeD_getPath(Node_D n);

/* Returns the number of child directories/files n has. */

size_t NodeD_getNumChildren(Node_D n);

/* Returns the number of child files n has. */

size_t NodeD_getNumFileChildren(Node_D n);

/* Returns the number of child directories n has. */

size_t NodeD_getNumDirChildren(Node_D n);

/* Returns 1 if n has a child directory with path,
   0 if it does not have such a child, and -1 if
   there is an allocation error during search.

   If n does have such a child, and childID is not NULL, store the
   child's identifier in *childID. If n does not have such a child,
   store the identifier that such a child would have in *childID. */

/*@unused@*/
int NodeD_hasDirChild(Node_D n, const char* path, size_t* childID);

/* Returns the child fileNode of n with identifier childID, if one
   exists, otherwise returns NULL. */

Node_F NodeD_getFileChild(Node_D n, size_t childID);

/* Returns the child dirNode of n with identifier childID, if one
   exists, otherwise returns NULL. */

Node_D NodeD_getDirChild(Node_D n, size_t childID);

/* Returns the parent node of n, if it exists,
   otherwise returns NULL */

Node_D NodeD_getParent(Node_D n);

/* Unlinks parent from its child fileNode child. child is 
   unchanged.

   Returns PARENT_CHILD_ERROR if child is not a child of parent,
   and SUCCESS otherwise. */

int NodeD_unlinkFileChild(Node_D parent, Node_F child);

/* Creates a new fileNode such that the new fileNode's path is dir 
   appended to n's path, separated by a slash, and that the new node
   has no children of its own. The new node's parent is n, and the
   new node is added as a child of n. The new node contains contents.
   The new file gets parameters contents and length. 

   (Reiterating for clarity: unlike with NodeF_create, parent *is*
   changed so that the link is bidirectional.)

   Returns SUCCESS upon completion, or:
   ALREADY_IN_TREE if parent already has a child with that path
   PARENT_CHILD_ERROR if the new child cannot otherwise be added */

int NodeD_addFileChild(Node_D parent, const char* dir, void* contents,
size_t length);

/* Creates a new dirNode such that the new dirNode's path is dir
   appended to n's path, separated by a slash, and that the new node
   has no children of its own. The new node's parent is n, and the
   new node is added as a child of n.

   (Reiterating for clarity: unlike with NodeD_create, parent *is*
   changed so that the link is bidirectional.)

   Returns the directory upon completion, or NULL if the operation
   could not be completed*/

Node_D NodeD_addDirChild(Node_D parent, const char* dir);

/* Returns a string representation for n,
   or NULL if there is an allocation error.

   Allocates memory for the returned string,
   which is then owned by client! */

char* NodeD_toString(Node_D n);

#endif
