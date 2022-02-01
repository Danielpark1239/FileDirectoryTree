/*--------------------------------------------------------------------*/
/* NodeD.c                                                          */
/* Author: John Hart and Daniel Park                                  */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "dynarray.h"
#include "NodeD.h"
#include "NodeF.h"
#include "checkerFT.h"

/*--------------------------------------------------------------------*/

/* A dirNode represents a directory in the tree. */
struct dirNode
{
   /* the full path of this directory */
   char* path;

   /* the parent directory of this directory
      NULL for the root of the directory tree */
   Node_D parent;

   /* the subdirectories of this directory
      stored in sorted order by pathname */
   DynArray_T dirChildren;

   /* the files stored of this directory
      stored in sorted order by pathname */
   DynArray_T fileChildren;
   
};

/*--------------------------------------------------------------------*/

/* Returns a path with contents n->path/dir
   or NULL if there is an allocation error.

   Allocates memory for the returned string,
   which is then owned by the caller. */
static char* NodeD_buildPath(Node_D n, const char* dir)
{
   char* path;

   assert(dir != NULL);

   if(n == NULL)
   {
      path = malloc(strlen(dir)+1);
   }
   else
   {
      path = malloc(strlen(n->path) + 1 + strlen(dir) + 1);
   }
   
   if(path == NULL)
   {
      return NULL;
   }
   *path = '\0';

   if(n != NULL)
   {
      strcpy(path, n->path);
      strcat(path, "/");
   }
   strcat(path, dir);

   return path;
}

/*--------------------------------------------------------------------*/

/* Given a parent node and a directory string dir, returns a new
   Node_D or NULL if any allocation error occurs in creating
   the node or its fields.

   The new structure is initialized to have its path as the parent's
   path (if it exists) prefixed to the directory string parameter,
   separated by a slash. It is also initialized with its parent link
   as the parent parameter value, but the parent itself is not changed
   to link to the new dirNode.  The children links are initialized but
   do not point to any children. */
static Node_D NodeD_create(const char* dir, Node_D parent)
{
   Node_D new;
   char* newPath;

   assert(parent == NULL || CheckerFT_Dir_isValid(parent));
   assert(dir != NULL);

   new = malloc(sizeof(struct dirNode));
   if(new == NULL)
      return NULL;
   
   newPath = NodeD_buildPath(parent, dir);
   if(newPath == NULL) {
      free(new);
      return NULL;
   }

   new->path = newPath;
   new->parent = parent;
   new->dirChildren = DynArray_new(0);
   if(new->dirChildren == NULL) {
      free(new->path);
      free(new);
      return NULL;
   }

   new->fileChildren = DynArray_new(0);
   if(new->fileChildren == NULL) {
      free(new->dirChildren);
      free(new->path);
      free(new);
      return NULL;
   }

   assert(CheckerFT_Dir_isValid(new));
   return new;
}

/*--------------------------------------------------------------------*/

/* Unlinks parent from its child dirNode child. child is
   unchanged.

   Returns PARENT_CHILD_ERROR if child is not a child of parent,
   and SUCCESS otherwise. */
static int NodeD_unlinkDirChild(Node_D parent, Node_D child)
{
   size_t i;

   assert(parent != NULL);
   assert(child != NULL);
   assert(CheckerFT_Dir_isValid(parent));
   assert(CheckerFT_Dir_isValid(child));

   if(DynArray_bsearch(parent->dirChildren, child, &i,
               (int (*)(const void*, const void*)) NodeD_compare) == 0)
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_Dir_isValid(child));
      return PARENT_CHILD_ERROR;
   }

   (void) DynArray_removeAt(parent->dirChildren, i);

   assert(CheckerFT_Dir_isValid(parent));
   assert(CheckerFT_Dir_isValid(child));
   return SUCCESS;
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
size_t NodeD_destroy(Node_D n)
{
   size_t i;
   size_t count = 0;
   Node_D d;
   Node_D parent;
   Node_F f;
   int result;

   assert(n != NULL);

   for(i = 0; i < DynArray_getLength(n->fileChildren);)
   {
      f = DynArray_get(n->fileChildren, i);
      result = NodeD_unlinkFileChild(n, f);
      assert(result == SUCCESS);

      result = NodeF_removeFile(f);
      f = NULL;
      assert(result == SUCCESS);

      count++;
   }
   for(i = 0; i < DynArray_getLength(n->dirChildren);)
   {
      d = DynArray_get(n->dirChildren, i);
      count += NodeD_destroy(d);
   }

   /* Unlink the parent */
   parent = NodeD_getParent(n);
   if(parent != NULL) {
      result = NodeD_unlinkDirChild(parent, n);
      assert(result == SUCCESS);
   }

   DynArray_free(n->dirChildren);
   DynArray_free(n->fileChildren);
   free(n->path);
   free(n);
   n = NULL;
   count++;

   return count;
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
int NodeD_compare(Node_D node1, Node_D node2)
{
   assert(node1 != NULL);
   assert(node2 != NULL);

   return strcmp(node1->path, node2->path);
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
const char* NodeD_getPath(Node_D n)
{
   if (n == NULL)
      return NULL;

   return n->path;
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
size_t NodeD_getNumChildren(Node_D n)
{
   if(n == NULL)
      return 0;

   return DynArray_getLength(n->dirChildren) +
      DynArray_getLength(n->fileChildren);
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
size_t NodeD_getNumFileChildren(Node_D n)
{
   if(n == NULL)
      return 0;

   return DynArray_getLength(n->fileChildren);
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
size_t NodeD_getNumDirChildren(Node_D n)
{
   if(n == NULL)
      return 0;

   return DynArray_getLength(n->dirChildren);
}

/*--------------------------------------------------------------------*/

/* Returns 1 if n has a child file with path,
   0 if it does not have such a child, and -1 if
   there is an allocation error during search.

   If n does have such a child, and childID is not NULL, store the
   child's identifier in *childID. If n does not have such a child,
   store the identifier that such a child would have in *childID. */
static int NodeD_hasFileChild(Node_D n, const char* path, size_t* childID)
{  
   size_t index;
   int result;
   Node_F checker;

   assert(n != NULL);
   assert(path != NULL);

   checker = NodeF_create(path, NULL, NULL, 0);
   if(checker == NULL) {
      return -1;
   }
   result = DynArray_bsearch(n->fileChildren, checker, &index,
                             (int (*)(const void*, const void*))
                             NodeF_compare);
   (void) NodeF_removeFile(checker);

   if(childID != NULL)
      *childID = index;

   return result;
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
int NodeD_hasDirChild(Node_D n, const char* path, size_t* childID)
{
   size_t index;
   int result;
   Node_D checker;

   assert(n != NULL);
   assert(path != NULL);

   checker = NodeD_create(path, NULL);
   if(checker == NULL) {
      return -1;
   }
   result = DynArray_bsearch(n->dirChildren, checker, &index,
                             (int (*)(const void*, const void*))
                             NodeD_compare);
   (void) NodeD_destroy(checker);

   if(childID != NULL)
      *childID = index;

   return result;
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
Node_F NodeD_getFileChild(Node_D n, size_t childID)
{
   if(n == NULL)
      return NULL;

   if(DynArray_getLength(n->fileChildren) > childID) {
      return DynArray_get(n->fileChildren, childID);
   }
   else {
      return NULL;
   }
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
Node_D NodeD_getDirChild(Node_D n, size_t childID)
{
   if(n == NULL)
      return NULL;

   if(DynArray_getLength(n->dirChildren) > childID) {
      return DynArray_get(n->dirChildren, childID);
   }
   else {
      return NULL;
   }
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
Node_D NodeD_getParent(Node_D n)
{
   if(n == NULL)
      return NULL;

   return n->parent;
}

/*--------------------------------------------------------------------*/

/* Makes fileNode a child of parent, if possible, and returns SUCCESS.
   This is not possible in the following cases:
   * parent already has a child with child's path,
     in which case: returns ALREADY_IN_TREE
   * child's path is not parent's path + / + directory,
     or the parent cannot link to the child,
     in which cases: returns PARENT_CHILD_ERROR */
static int NodeD_linkFileChild(Node_D parent, Node_F child)
{
   size_t i;
   char* rest;

   assert(parent != NULL);
   assert(child != NULL);
   assert(CheckerFT_Dir_isValid(parent));

   /* Check if child is already in tree */
   if(NodeD_hasFileChild(parent, NodeF_getPath(child), NULL))
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_File_isValid(child));
      return ALREADY_IN_TREE;
   }

   /* Checks that child's path includes exactly parent's path */
   i = strlen(parent->path);
   if(strncmp(NodeF_getPath(child), parent->path, i))
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_File_isValid(child));
      return PARENT_CHILD_ERROR;
   }

   /* Checks that child's path is a valid path string */
   rest = (char*) NodeF_getPath(child)+ i;
   if(strlen(NodeF_getPath(child)) >= i && rest[0] != '/')
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_File_isValid(child));
      return PARENT_CHILD_ERROR;
   }

   /* Makes sure child's path doesn't contain another '/' at the end */
   rest++;
   if(strstr(rest, "/") != NULL)
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_File_isValid(child));
      return PARENT_CHILD_ERROR;
   }

   NodeF_linkFile(child, parent);

   /* Checks if the file is already in the tree */
   if(DynArray_bsearch(parent->fileChildren, child, &i,
               (int (*)(const void*, const void*)) NodeF_compare) == 1)
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_File_isValid(child));
      return ALREADY_IN_TREE;
   }

   /* Checks if file was successfully linked into tree */
   if(DynArray_addAt(parent->fileChildren, i, child) == TRUE)
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_File_isValid(child));
      return SUCCESS;
   }

   /* If it was not successfully linked, some error occurred */
   else
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_File_isValid(child));
      return PARENT_CHILD_ERROR;
   }
}


/*--------------------------------------------------------------------*/

/* Makes dirNode a child of parent, if possible, and returns SUCCESS.
   This is not possible in the following cases:
   * parent already has a child with child's path,
     in which case: returns ALREADY_IN_TREE
     * child's path is not parent's path + / + directory,
     or the parent cannot link to the child,
     in which cases: returns PARENT_CHILD_ERROR */
static int NodeD_linkDirChild(Node_D parent, Node_D child)
{
   size_t i;
   char* rest;

   assert(parent != NULL);
   assert(child != NULL);
   assert(CheckerFT_Dir_isValid(parent));
   assert(CheckerFT_Dir_isValid(child));

   /* Check if child is already in tree */
   if(NodeD_hasDirChild(parent, child->path, NULL))
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_Dir_isValid(child));
      return ALREADY_IN_TREE;
   }

   /* Checks that child's path includes exactly parent's path */
   i = strlen(parent->path);
   if(strncmp(child->path, parent->path, i))
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_Dir_isValid(child));
      return PARENT_CHILD_ERROR;
   }

   /* Checks that child's path is a valid path string */
   rest = child->path + i;
   if(strlen(child->path) >= i && rest[0] != '/')
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_Dir_isValid(child));
      return PARENT_CHILD_ERROR;
   }

   /* Makes sure child's path doesn't contain another '/' at the end */
   rest++;
   if(strstr(rest, "/") != NULL)
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_Dir_isValid(child));
      return PARENT_CHILD_ERROR;
   }
   
   child->parent = parent;

   /* Checks if the directory is already in the tree */
   if(DynArray_bsearch(parent->dirChildren, child, &i,
               (int (*)(const void*, const void*)) NodeD_compare) == 1)
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_Dir_isValid(child));
      return ALREADY_IN_TREE;
   }

   /* Checks if file was successfully linked into tree */
   if(DynArray_addAt(parent->dirChildren, i, child) == TRUE)
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_Dir_isValid(child));
      return SUCCESS;
   }

   /* If it was not successfully linked, some error occurred */
   else
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_Dir_isValid(child));
      return PARENT_CHILD_ERROR;
   }
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
int NodeD_unlinkFileChild(Node_D parent, Node_F child)
{
   size_t i = 0;

   assert(parent != NULL);
   assert(child != NULL);
   assert(CheckerFT_Dir_isValid(parent));
   assert(CheckerFT_File_isValid(child));

   if(DynArray_bsearch(parent->fileChildren, child, &i,
               (int (*)(const void*, const void*)) NodeF_compare) == 0)
   {
      assert(CheckerFT_Dir_isValid(parent));
      assert(CheckerFT_File_isValid(child));
      return PARENT_CHILD_ERROR;
   }

   (void) DynArray_removeAt(parent->fileChildren, i);

   assert(CheckerFT_Dir_isValid(parent));
   assert(CheckerFT_File_isValid(child));
   return SUCCESS;
   
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
int NodeD_addFileChild(Node_D parent, const char* dir, void* contents,
size_t length)
{
   Node_F new;
   int result;

   assert(parent != NULL);
   assert(dir != NULL);
   assert(CheckerFT_Dir_isValid(parent));

   new = NodeF_create(dir, parent, contents, length);
   if(new == NULL)
   {
      assert(CheckerFT_Dir_isValid(parent));
      return PARENT_CHILD_ERROR;
   }

   result = NodeD_linkFileChild(parent, new);
   if(result != SUCCESS)
   {
      (void) NodeF_removeFile(new);
   }
   else
   {
      assert(CheckerFT_File_isValid(new));
   }

   assert(CheckerFT_Dir_isValid(parent));
   return result;
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
Node_D NodeD_addDirChild(Node_D parent, const char* dir)
{
   Node_D new;
   int result;

   assert(dir != NULL);
   assert(CheckerFT_Dir_isValid(parent));

   new = NodeD_create(dir, parent);
   if(new == NULL)
   {
      assert(CheckerFT_Dir_isValid(parent));
      return NULL;
   }
   
   if(parent != NULL) {
      result = NodeD_linkDirChild(parent, new);
      if(result != SUCCESS) {
         (void)NodeD_destroy(new);
         return NULL;
      }
   }

   assert(CheckerFT_Dir_isValid(parent));
   assert(CheckerFT_Dir_isValid(new));
   return new;
}

/*--------------------------------------------------------------------*/

/* See NodeD.h for specification. */
char* NodeD_toString(Node_D n)
{
   char* copyPath;

   assert(n != NULL);

   copyPath = malloc(strlen(n->path)+1);
   if(copyPath == NULL)
   {
      return NULL;
   }
   else
   {
      return strcpy(copyPath, n->path);
   }
}
