/*--------------------------------------------------------------------*/
/* ft.c                                                               */
/* Author: Daniel Park and John Hart                                  */
/*--------------------------------------------------------------------*/
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "dynarray.h"
#include "ft.h"
#include "NodeF.h"
#include "NodeD.h"
#include "checkerFT.h"

/* A File Tree is an AO with 3 state variables: */
/* a flag for if it is in an initialized state (TRUE) or not (FALSE) */
static boolean isInitialized;
/* a pointer to the root node in the hierarchy */
static Node_D root;
/* a counter of the number of nodes in the hierarchy */
static size_t count;


/* Starting at the parameter curr, traverses as far down the hierarchy
as possible while still matching the path parameter.

Returns a pointer to the farthest matching directory down that
path, or NULL if there is no directory in curr's hierarchy that 
matches a prefix of the path. */
static Node_D FT_traverseDirPathFrom(char* path, Node_D curr) {
   Node_D found;
   size_t i;

   assert(path != NULL);
   if(curr == NULL) 
      return NULL;

   /* If the current node has the same path as the path parameter,
   return curr */
   else if(!strcmp(path, NodeD_getPath(curr))) 
      return curr;

   /* If the path parameter contains the current node's path, search
   through all curr's children to find a match */
   else if(!strncmp(path, NodeD_getPath(curr), 
   strlen(NodeD_getPath(curr)))) {
      for(i = 0; i < NodeD_getNumDirChildren(curr); i++) {
         found = FT_traverseDirPathFrom(path,
         NodeD_getDirChild(curr, i));
         if(found != NULL) return found;
      }
      /* If no match is found, return curr */
      return curr;
   }
   /* If the paths don't match, return NULL */
   return NULL;
}

/* Starting at the parameter curr, traverses as far down the hierarchy
as possible while still matching the match parameter.

Returns a pointer to the farthest matching file down that path, or NULL
if there is no file in curr's hierarchy that matches a prefix of the
path. */
static Node_F FT_traverseFilePathFrom(char* path, Node_D curr) {
   Node_D directory;
   Node_F child;
   size_t i;
   assert(path != NULL);
   assert(CheckerFT_Dir_isValid(curr));

   /* Reach the farthest directory that matches a prefix of the path */
   directory = FT_traverseDirPathFrom(path, curr);

   /* If there is no directory that matches a prefix of the path,
   there is matching file. */
   if (directory == NULL)
      return NULL;

   /* Search the farthest directory's files for a match */
   for(i = 0; i < NodeD_getNumFileChildren(directory); i++) {
      child = NodeD_getFileChild(directory, i);
      /* If a file exists as a proper prefix of the path, return it */
      if(!strncmp(path, NodeF_getPath(child), 
      strlen(NodeF_getPath(child))) && child != NULL) {
         return child;
      }
   } 

   /* If no match is found, return NULL */
   return NULL;
}

/* Starting at the root, traverse as far down the hierarchy as possible
while still matching the path parameter.

Returns a pointer to the farthest matching directory down that
path, or NULL if there is no directory in the root's hierarchy 
that matches a prefix of the path. */
static Node_D FT_traverseDirPath(char* path) {
    assert(path != NULL);
    return FT_traverseDirPathFrom(path, root);
}

/* Starting at the root, traverse as far down the hierachy as possible
while still matching the path parameter.

Returns a pointer to the farthest matching file down that path, or NULL 
if there is no directory in the root's hierarchy that matches a prefix
of that path. */
static Node_F FT_traverseFilePath(char* path) {
   assert(path != NULL);
   return FT_traverseFilePathFrom(path, root);
}

/*
   Destroys the entire hierarchy of nodes rooted at curr,
   including curr itself.
*/
static void FT_removeDirPathFrom(Node_D curr) {
   if(curr != NULL) {

      if(curr == root) {
         count -= NodeD_destroy(curr);
         root = NULL;
      }
      
      else count -= NodeD_destroy(curr);
   }
}

/*
   Inserts a new directory into the tree rooted at parent, or, if
   parent is NULL, at the root of the data structure.

   If a node representing path already exists, returns ALREADY_IN_TREE

   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR

   If there is an error linking any of the new nodes,
   returns PARENT_CHILD_ERROR

   Otherwise, returns SUCCESS
*/
static int FT_insertDirRestOfPath(char* path, Node_D parent) {
   char* copyPath;
   char* restPath = path;
   char* dirToken;
   Node_D newNode;
   size_t newCount = 0;

   assert(path != NULL);

   /* If curr is NULL but it isn't the root node, 
      then there is something wrong with the path */
   if(parent == NULL)
   {
      if(root != NULL)
      {
         return CONFLICTING_PATH;
      }
   }
   /* Check if node is already in tree */
   else if(!strcmp(path, NodeD_getPath(parent))) {
      return ALREADY_IN_TREE;
   }

   if(parent != NULL)
      restPath += (strlen(NodeD_getPath(parent)) + 1);

   /* Make sure there's no memory error */
   copyPath = malloc(strlen(restPath)+1);
   if(copyPath == NULL)
   {
      return MEMORY_ERROR;
   }
   strcpy(copyPath, restPath);
   dirToken = strtok(copyPath, "/");

   while(dirToken != NULL)
   {
      newNode = NodeD_addDirChild(parent, dirToken);

      /* Check for memory error */
      if(newNode == NULL)
      {
         free(copyPath);
         return MEMORY_ERROR;
      }

      /* If the parent is NULL, set the root */
      if(parent == NULL && root == NULL) {
         root = newNode;
      }

      count++;
      parent = newNode;
      dirToken = strtok(NULL, "/");
   }

   free(copyPath);
   count += newCount;
   return SUCCESS;
}

/* see ft.h for specification */
int FT_insertDir(char *path)
{
   Node_D curr;
   Node_F file;
   int result;

   assert(CheckerFT_isValid(isInitialized,root,count));
   assert(path != NULL);

   if(isInitialized == FALSE)
      return INITIALIZATION_ERROR;

   /* Makes sure there isn't a file with the same path */
   file = FT_traverseFilePath(path);

   if(file != NULL) {
      /* If the file is already in the tree, return ALREADY_IN_TREE */
      if(!strcmp(NodeF_getPath(file), path))
         return ALREADY_IN_TREE;

      /* If the file exists and it's a proper prefix of path,
      return NOT_A_DIRECTORY */
      return NOT_A_DIRECTORY;
   }
   
   /* Makes sure there is no conflict with the path */
   curr = FT_traverseDirPath(path);

   if(curr == NULL && root != NULL)
      return CONFLICTING_PATH;

   if(curr != NULL) {
      if(!strcmp(NodeD_getPath(curr), path))
         return ALREADY_IN_TREE;
   }
   
   result = FT_insertDirRestOfPath(path, curr);
   assert(CheckerFT_isValid(isInitialized,root,count));
   return result;
}

/* see ft.h for specification. */
boolean FT_containsDir(char *path)
{
   Node_D curr;
   boolean result;

   assert(CheckerFT_isValid(isInitialized,root,count));
   assert(path != NULL);

   if(!isInitialized)
   {
      return FALSE;
   }

   curr = FT_traverseDirPath(path);


   if(curr == NULL)
      result = FALSE;
   else if(strcmp(path, NodeD_getPath(curr)))
      result = FALSE;
   else
      result = TRUE;

   assert(CheckerFT_isValid(isInitialized,root,count));
   return result;
}

/* Removes the directory hierarchy rooted at path starting from curr.
If curr is the data structure's root, root becomes NULL.

Returns NO_SUCH_PATH if curr is not the node for path, and SUCCESS
otherwise. */
static int FT_rmDirPathAt(char* path, Node_D curr) {
   assert(path != NULL);
   assert(curr != NULL);

   if(!strcmp(path, NodeD_getPath(curr))) {
      FT_removeDirPathFrom(curr);
      return SUCCESS;
   }
   return NO_SUCH_PATH;
}

/* see ft.h for specification */
int FT_rmDir(char *path)
{
   Node_D curr;
   Node_F file;
   int result;

   assert(CheckerFT_isValid(isInitialized,root,count));
   assert(path != NULL);

   if(!isInitialized)
      return INITIALIZATION_ERROR;

   if(root == NULL)
      return NO_SUCH_PATH;

   /* If the path exists, but is a file, return NOT_A_DIRECTORY */
   file = FT_traverseFilePath(path);
   if(file != NULL)
      return NOT_A_DIRECTORY;

   /* If no directory is found, return NO_SUCH_PATH */
   curr = FT_traverseDirPath(path);
   if(curr == NULL)
      result =  NO_SUCH_PATH;

   /* If a directory is found, but does not match the path, return
   NO_SUCH_PATH */
   if(!strcmp(NodeD_getPath(curr), path))
   {
      result = FT_rmDirPathAt(path, curr);
   }

   else result = NO_SUCH_PATH;

   assert(CheckerFT_isValid(isInitialized,root,count));
   return result;
}

/* Inserts a new file into the tree rooted at parent that stores
   given contents and the file's path as well as the file length 

   If there is an allocation error in creating any of the new nodes or
   their fields, returns MEMORY_ERROR

   If there is an error linking any of the new nodes,
   returns PARENT_CHILD_ERROR

   Otherwise, returns SUCCESS */
static int FT_insertFileRestOfPath(char* path, Node_D parent,
void* contents, size_t length) {
   int i;
   char* restPath = path;
   char* dirToken;
   Node_D dir;

   assert(path != NULL);
   assert(parent != NULL);

   restPath += (strlen(NodeD_getPath(parent)) + 1);

   /* Make sure there's no memory error */
   dirToken = malloc(strlen(restPath) + 1);
   if(dirToken == NULL)
      return MEMORY_ERROR;

   strcpy(dirToken, restPath);
   dirToken = strtok(dirToken, "/");

   if(dirToken == NULL)
      return PARENT_CHILD_ERROR;

   /* Add directories if needed */
   while(dirToken != NULL && strstr((const char*)dirToken, "/") != NULL) 
   {
      dir = NodeD_addDirChild(parent, (const char*)dirToken);
      if(dir == NULL) {
         free(dirToken);
         return PARENT_CHILD_ERROR;
      }
      count++;
      dirToken = strtok(NULL, "/");
   }

   /* Add the file */
   i = NodeD_addFileChild(parent, (const char*)dirToken, contents,
   length);
   if(i != SUCCESS) {
      free(dirToken);
      return PARENT_CHILD_ERROR;
   }
   count++;
   free(dirToken);
   return SUCCESS;
}

/* see ft.h for specification */
int FT_insertFile(char *path, void *contents, size_t length) {
   Node_D directory;
   Node_D parent;
   Node_F file;
   Node_F newFile;
   size_t destroyedCount;
   char *dirPath;
   int result;

   assert(CheckerFT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(isInitialized == FALSE)
      return INITIALIZATION_ERROR;

   /* Can't insert a file into the root */
   if(root == NULL)
      return CONFLICTING_PATH;

   /* Find the farthest directory down the hierarchy */
   directory = FT_traverseDirPath(path);
   if(directory == NULL)
      return CONFLICTING_PATH;

   /* Find the farthest file down the hierarchy */
   file = FT_traverseFilePath(path);

   if(file != NULL) {
      /* If the file is already in the tree, return ALREADY_IN_TREE */
      if(!strcmp(NodeF_getPath(file), path))
         return ALREADY_IN_TREE;

      /* If the file exists and it's a proper prefix of path,
      return NOT_A_DIRECTORY */
      return NOT_A_DIRECTORY;
   }
   
   /* If the directory is already in the tree, return ALREADY_IN_TREE */
   dirPath = (char*)NodeD_getPath(directory);
   if(!strcmp(dirPath, path))
      return ALREADY_IN_TREE;

   /* Insert a directory at the path */
   result = FT_insertDir(path);
   if(result != SUCCESS)
      return MEMORY_ERROR;
   
   /* Set directory to the newly added node */
   directory = FT_traverseDirPath(path);
   assert(!strcmp(NodeD_getPath(directory), path)); /*CHECK */

   /* Remove the last directory and insert a file */
   parent = NodeD_getParent(directory);
   destroyedCount = NodeD_destroy(directory);

   /* CHECK */
   assert(destroyedCount == 1);
   count = count - destroyedCount;

   result = FT_insertFileRestOfPath(path, parent, contents, length);
   if(result != SUCCESS) 
      return MEMORY_ERROR;
   
   assert(CheckerFT_isValid(isInitialized, root, count));

   /* EXTRA CHECKS JUST TO BE SAFE */
   newFile = FT_traverseFilePath(path);
   assert(!strcmp(NodeF_getPath(newFile), path));
   assert(CheckerFT_File_isValid(newFile));
   return SUCCESS;
}

/* see ft.h for specification */
boolean FT_containsFile(char *path) {
   Node_F file;

   assert(CheckerFT_isValid(isInitialized, root, count));
   assert(path != NULL);

   /* If a file that's a proper prefix of the path doesn't
   exist, return false */
   file = FT_traverseFilePath(path);
   if(file == NULL)
      return FALSE;
   
   /* If the found file doesn't have the given path, return false */
   if(strcmp(NodeF_getPath(file), path))
      return FALSE;

   assert(CheckerFT_isValid(isInitialized, root, count));
   return TRUE;
}

/* see ft.h for specification */
int FT_rmFile(char *path) {
   Node_D directory;
   Node_D parent;
   Node_F file;

   assert(CheckerFT_isValid(isInitialized, root, count));
   assert(path != NULL);

   if(isInitialized == FALSE)
      return INITIALIZATION_ERROR;

   /* If there is no directory that's a proper prefix of path, return
   NO_SUCH_PATH */
   directory = FT_traverseDirPath(path);
   if(directory == NULL)
      return NO_SUCH_PATH;

   /* If the path is a directory, return NOT_A_FILE */
   if(!strcmp(NodeD_getPath(directory), path))
      return NOT_A_FILE;

   /* If there is no file that's a proper prefix of path, return 
   NO_SUCH_PATH */
   file = FT_traverseFilePath(path);
   if(file == NULL)
      return NO_SUCH_PATH;
   
   /* If the found file doesn't match the path, return NO_SUCH_PATH */
   if(strcmp(NodeF_getPath(file), path))
      return NO_SUCH_PATH;

   /* If the operation fails, return an error */
   parent = NodeF_getDirectory(file);
   assert(parent != NULL);
   NodeD_unlinkFileChild(parent, file);
   (void)NodeF_removeFile(file);

   assert(CheckerFT_isValid(isInitialized, root, count));
   count--;
   return SUCCESS;
}

/* see ft.h for specification */
void *FT_getFileContents(char *path) {
   Node_D directory;
   Node_F file;

   assert(path != NULL);
   assert(CheckerFT_isValid(isInitialized, root, count));

   if(isInitialized == FALSE)
      return NULL;

   /* If a directory doesn't exist as a proper prefix of the path,
   return NULL */
   directory = FT_traverseDirPath(path);
   if(directory == NULL)
      return NULL;
   if(!strcmp(NodeD_getPath(directory), path))
      return NULL;

   /* If the found file doesn't match the path, return NULL */
   file = FT_traverseFilePath(path);
   if(file == NULL)
      return NULL;
   if(strcmp(NodeF_getPath(file), path))
      return NULL;

   assert(CheckerFT_isValid(isInitialized, root, count));
   return NodeF_getContents(file);
}

/* see ft.h for specification */
void *FT_replaceFileContents(char *path, void *newContents,
size_t newLength) {
   Node_D directory;
   Node_F file;
   void* oldContents;

   assert(path != NULL);
   assert(CheckerFT_isValid(isInitialized, root, count));

   if(isInitialized == FALSE)
      return NULL;

   /* If a directory doesn't exist as a proper prefix of the path,
   return NULL */
   directory = FT_traverseDirPath(path);
   if(directory == NULL)
      return NULL;
   if(!strcmp(NodeD_getPath(directory), path))
      return NULL;

   /* If the found file doesn't match the path, return NULL */
   file = FT_traverseFilePath(path);
   if(file == NULL)
      return NULL;
   if(strcmp(NodeF_getPath(file), path))
      return NULL;

   oldContents = NodeF_getContents(file);
   NodeF_replaceContents(file, newContents, newLength);

   assert(CheckerFT_isValid(isInitialized, root, count));
   assert(CheckerFT_File_isValid(file));

   return oldContents;
}

/* see ft.h for specification */
int FT_stat(char *path, boolean *type, size_t *length) {
   Node_D directory;
   Node_F file;

   assert(CheckerFT_isValid(isInitialized, root, count));
   assert(path != NULL);
   assert(type != NULL);
   assert(length != NULL);

   if(isInitialized == FALSE)
      return INITIALIZATION_ERROR;

   directory = FT_traverseDirPath(path);
   if(directory == NULL)
      return NO_SUCH_PATH;

   /* If the path is a directory: */
   if(!strcmp(NodeD_getPath(directory), path)) {
      assert(CheckerFT_Dir_isValid(directory));
      *type = FALSE;
      return SUCCESS;

   }

   file = FT_traverseFilePathFrom(path, directory);
   if(file == NULL)
      return NO_SUCH_PATH;

   /* If the path is a file: */
   if(!strcmp(NodeF_getPath(file), path)) {
      assert(CheckerFT_File_isValid(file));
      *type = TRUE;
      *length = NodeF_getLength(file);
      return SUCCESS;
   }  

   /* If the path is not found */
   return NO_SUCH_PATH;
}

/* see ft.h for specification. */
int FT_init(void) {
   assert(CheckerFT_isValid(isInitialized, root, count));

   if (isInitialized == TRUE)
      return INITIALIZATION_ERROR;

   isInitialized = TRUE;
   count = 0;
   root = NULL;

   assert(CheckerFT_isValid(isInitialized, root, count));
   return SUCCESS;
}

/* see ft.h for specification */
int FT_destroy(void) {
   assert(CheckerFT_isValid(isInitialized, root, count));

   if (isInitialized == FALSE)
      return INITIALIZATION_ERROR;

   FT_removeDirPathFrom(root);
   isInitialized = FALSE;
   root = NULL;
   assert(count == 0);

   assert(CheckerFT_isValid(isInitialized, root, count));
   return SUCCESS;
}

/* Performs a pre-order traversal of the tree rooted at n, going over
   each directory child and file child to DynArray_T d and DynArray_T f
   beginning at index i. Returns the total number of nodes in the
   FT. */
static size_t FT_preOrderTraversal(Node_D n, DynArray_T d, size_t i)
{
   size_t c;

   assert(d != NULL);

   if(n == root)
   {
      (void) DynArray_set(d, i, NodeD_getPath(n));
   }
   if(n != NULL) {
      i++;
      for(c = 0; c < NodeD_getNumFileChildren(n); c++)
      {
         (void) DynArray_set(d, i,
                             NodeF_getPath(NodeD_getFileChild(n, c)));
         i++;
      }
      for(c = 0; c < NodeD_getNumDirChildren(n); c++)
      {
         (void) DynArray_set(d, i,
                             NodeD_getPath(NodeD_getDirChild(n, c)));
         i = FT_preOrderTraversal(NodeD_getDirChild(n, c), d, i);
      }
   }
   return i;
}

/* Alternate version of strlen that uses pAcc as an in-out parameter
   to accumulate a string length, rather than returning the length of
   str, and also always adds one more in addition to str's length. */
static void FT_strlenAccumulate(char* str, size_t* pAcc) {
   assert(pAcc != NULL);
   assert(str != NULL);
   
   *pAcc += (strlen(str) + 1);
}

/* Alternate version of strcat that inverts the typical argument
   order, appending str onto acc, and also always adds a newline at
   the end of the concatenated string. */
static void FT_strcatAccumulate(char* str, char* acc) {
   assert(acc != NULL);
   assert(str != NULL);

   strcat(acc, str); 
   strcat(acc, "\n");
}

/* see ft.h for specification */
char *FT_toString(void) {
   DynArray_T nodes;
   size_t totalStrlen = 1;
   char* result = NULL;

   assert(CheckerFT_isValid(isInitialized, root, count));

   if(isInitialized == FALSE)
      return NULL;

   if(root == NULL) {
      result = malloc(sizeof(char));
      if(result == NULL) {
         assert(CheckerFT_isValid(isInitialized, root, count));
         return NULL;
      }
      *result = '\0';
      return result;
   }

   nodes = DynArray_new(count);
   (void) FT_preOrderTraversal(root, nodes, 0);

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strlenAccumulate,
                (void*) &totalStrlen);

   result = malloc(totalStrlen);
   if(result == NULL) {
      DynArray_free(nodes);
      assert(CheckerFT_isValid(isInitialized,root,count));
      return NULL;
   }
   *result = '\0';

   DynArray_map(nodes, (void (*)(void *, void*)) FT_strcatAccumulate,
                (void *) result);

   DynArray_free(nodes);
   assert(CheckerFT_isValid(isInitialized,root,count));
   return result;
}
