/*--------------------------------------------------------------------*/
/* NodeF.c                                                         */
/* Author: Daniel Park and John Hart                                  */
/*--------------------------------------------------------------------*/

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "NodeF.h"
#include "checkerFT.h"

/* A fileNode structure represents a file in the file tree */
struct fileNode {
    /* Contents */
    void* contents;

    /* Length of contents */
    size_t length;

    /* the full path of this file */
    char* path;

    /* the parent directory of this file */
    Node_D directory;
};

/* returns a path with contents n->path/dir
  or NULL if there is an allocation error.

  Allocates memory for the returned string,
  which is then owned by the caller. */
static char* NodeF_buildPath(Node_D n, const char* dir) {
    char* newPath;

    assert(dir != NULL);

    if(n == NULL)
        newPath = malloc(strlen(dir)+1);
    else
        newPath = malloc(strlen(NodeD_getPath(n)) + 1 + strlen(dir) + 1);

   if(newPath == NULL)
        return NULL;
   *newPath = '\0';

   if(n != NULL) {
        strcpy(newPath, NodeD_getPath(n));
        strcat(newPath, "/");
   }
   strcat(newPath, dir);

   return newPath;
}

/* see NodeF.h for specification */
Node_F NodeF_create(const char* path, Node_D directory, void* contents,
size_t length) {
   Node_F new;

   assert(directory == NULL || CheckerFT_Dir_isValid(directory));
   assert(path != NULL);

   new = malloc(sizeof(struct fileNode));
   if(new == NULL)
      return NULL;


   new->path = NodeF_buildPath(directory, path);
   if(new->path == NULL) { 
      free(new);
      new = NULL;
      return NULL;
   }

   new->directory = directory;
   new->contents = contents;
   new->length = length;

   return new;
}

/* see NodeF.h for specification */
int NodeF_compare(Node_F file1, Node_F file2) {
    assert(file1 != NULL);
    assert(file2 != NULL);

    return strcmp(file1->path, file2->path);
}

/* see NodeF.h for specification */
const char* NodeF_getPath(Node_F n) {
    assert(n != NULL);
    
    return n->path;
}

/* see NodeF.h for specification */
Node_D NodeF_getDirectory(Node_F n) {
    assert(n != NULL);

    return n->directory;
}

/* see NodeF.h for specification */
size_t NodeF_getLength(Node_F n) {
    assert(n != NULL);

    return n->length;
}

/* see NodeF.h for specification */
void* NodeF_getContents(Node_F n) {
    assert(n != NULL);

    return n->contents;
}

/* see NodeF.h for specification */
void NodeF_replaceContents(Node_F n, void* newContents, 
size_t newLength) {
    assert(n != NULL);

    n->contents = newContents;
    n->length = newLength;
}

/* see NodeF.h for specification */
int NodeF_linkFile(Node_F file, Node_D directory) {
    assert(CheckerFT_Dir_isValid(directory));
    assert(CheckerFT_File_isValid(file));

    file->directory = directory;
    return SUCCESS;
}


/* see NodeF.h for specification. */
int NodeF_removeFile(Node_F file) {

    free(file->path);
    file->path = NULL;
    free(file);
    file = NULL;

    return SUCCESS;
}

/* see NodeF.h for specification. */
char* NodeF_toString(Node_F n) {
    char* copyPath;

    assert(n != NULL);

    copyPath = malloc(strlen(n->path)+1);
    if(copyPath == NULL) 
        return NULL;
    else 
        return strcpy(copyPath, n->path);
}