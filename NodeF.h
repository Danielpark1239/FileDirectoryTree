/*--------------------------------------------------------------------*/
/* NodeF.h                                                         */
/* Author: Daniel Park and John Hart                                  */
/*--------------------------------------------------------------------*/

#ifndef FILENODE_INCLUDED
#define FILENODE_INCLUDED

#include <stddef.h>
#include "a4def.h"
#include "nodes.h"

/*--------------------------------------------------------------------*/

/* Given a directory, a path string path, and contents,
returns a new Node_F or NULL if any allocation error occurs 
in creating the file or its fields.

The new structure is initialized to have its path as the directory's
path prefixed to the file path parameter, separated by a slash. It is 
also initialized with its directory link as the directory parameter 
value, but the directory itself is not changed to link to the new node.
The contents and length are passed to the file. */

Node_F NodeF_create(const char* path, Node_D directory, void* contents,
size_t length);

/*--------------------------------------------------------------------*/

/* Compares file1 and file2 based on their paths. Returns <0, 0, or >0 
if node1's path is less than, equal to, or greater than node2's path, 
respectively. */

int NodeF_compare(Node_F file1, Node_F file2);

/*--------------------------------------------------------------------*/

/* Returns n's path. */
const char* NodeF_getPath(Node_F n);

/*--------------------------------------------------------------------*/

/* Returns the parent directory of n. */
Node_D NodeF_getDirectory(Node_F n);

/*--------------------------------------------------------------------*/

/* Returns the length of n's contents. */
size_t NodeF_getLength(Node_F n);

/*--------------------------------------------------------------------*/

/* Returns the contents of n. */
void* NodeF_getContents(Node_F n);

/*--------------------------------------------------------------------*/

/* Replace the contents of n with newContents and newLength */
void NodeF_replaceContents(Node_F n, void* newContents, 
size_t newLength);

/*--------------------------------------------------------------------*/

/* Links file n to a parent directory, replacing its past parent link.
Doesn't modify the parent. Returns SUCCESS upon completion or NULL if 
the operation cannot be completed. */
int NodeF_linkFile(Node_F file, Node_D directory);

/*--------------------------------------------------------------------*/

/* Removes the given file from the file tree. Leaves the parent 
unchanged. 

Returns SUCCESS upon completion or NULL if the file doesn't exist or 
it can otherwise not be removed. */
int NodeF_removeFile(Node_F file);

/*--------------------------------------------------------------------*/

/* Returns a string representation for n,  or NULL if there is an 
allocation error.

Allocates memory for the returned string, which is then owned by 
client. */
char* NodeF_toString(Node_F n);

/*--------------------------------------------------------------------*/

#endif