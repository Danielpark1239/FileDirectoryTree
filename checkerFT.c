/*--------------------------------------------------------------------*/
/* checkerFT.c                                                        */
/* Author: Daniel Park and John Hart                                  */
/*--------------------------------------------------------------------*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "dynarray.h"
#include "checkerFT.h"

/* TO DO:
- make sure Directory's files are in sorted order --> check!
- update children counter stuff here --> check!
- make sure total children add up to getnumDir and getnumFile --> check!
- and the numbers should be right for each category --> check! */

/* see checkerFT.h for specification */
boolean CheckerFT_Dir_isValid(Node_D n) {
   Node_D parent;
   const char* npath;
   const char* ppath;
   const char* rest;
   size_t i;
  
   /* Initial n->path checks: */
   /* A NULL pointer is not a valid path */
   if(n != NULL) {
      if(NodeD_getPath(n) == NULL) {
         fprintf(stderr, "A node's path is a NULL pointer\n");
         return FALSE;
      }
   } 
   
   /* n->parent checks and more n->path checks: */
   parent = NodeD_getParent(n);
   if(parent != NULL) {
      npath = NodeD_getPath(n);

      /* Parent's path must be prefix of n's path */
      ppath = NodeD_getPath(parent);
      i = strlen(ppath);
      if(strncmp(npath, ppath, i)) {
         fprintf(stderr, "P's path is not a prefix of C's path\n");
         return FALSE;
      }
      /* n's path after parent's path + '/' must have no further
      '/' characters */
      rest = npath + i;
      rest++;
      if(strstr(rest, "/") != NULL) {
         fprintf(stderr, "C's path has grandchild of P's path\n");
         return FALSE;
      }
   }

   /* n->dirChildren checks: */
   if(NodeD_getNumDirChildren(n) == 1)
   {
      /* Children must not be a NULL pointer/number of children
         must equal length of n->dirChildren */
      if(NodeD_getDirChild(n, 0) == NULL)
      {
         fprintf(stderr, "n's child is a NULL pointer\n");
         return FALSE;
      }
   }
   /* Check each child if n has directory children. */
   else if(NodeD_getNumDirChildren(n) != 0)
   {
      for(i = 0; i < NodeD_getNumDirChildren(n) - 1; i++)
      {
         /* Children must not be a NULL pointer/number of children
            must equal length of n->dirChildren */
         if(NodeD_getDirChild(n, i) == NULL ||
            NodeD_getDirChild(n, i + 1) == NULL)
         {
            fprintf(stderr, "One of n's children is a NULL pointer\n");
            return FALSE;
         }
         /* Children must be in sorted order */
         else if(NodeD_compare(NodeD_getDirChild(n, i),
                              NodeD_getDirChild(n, i + 1)) > 0)
         {
            fprintf(stderr, "n's children are not in sorted order\n");
            return FALSE;
         }
     }
   }

   if(NodeD_getNumFileChildren(n) == 1)
   {
      /* Children must not be a NULL pointer/number of children
         must equal length of n->fileChildren */
      if(NodeD_getFileChild(n, 0) == NULL)
      {
         fprintf(stderr, "One of n's children is a NULL pointer\n");
         return FALSE;
      }
   }
   /* If n has file children, check them */
   else if(NodeD_getNumFileChildren(n) != 0)
   {
      for(i = 0; i < NodeD_getNumFileChildren(n) - 1; i++)
      {
         /* Children must not be a NULL pointer/number of children
            must equal length of n->fileChildren */
         if(NodeD_getFileChild(n, i) == NULL ||
            NodeD_getFileChild(n, i + 1) == NULL)
         {
            fprintf(stderr, "One of n's children is a NULL pointer\n");
            return FALSE;
         }
         /* Children must be in sorted order */
         else if(NodeF_compare(NodeD_getFileChild(n, i),
                               NodeD_getFileChild(n, i + 1)) > 0)
         {
            fprintf(stderr, "n's children are not in sorted order\n");
            return FALSE;
         }
      }
   }

   /* Number of directory children and file children must equal
      number of all children*/
   if(NodeD_getNumDirChildren(n) + NodeD_getNumFileChildren(n) !=
      NodeD_getNumChildren(n))
   {
      fprintf(stderr, "n has an incorrect number of children\n");
      return FALSE;
   }
   
   return TRUE;
}

/* see checkerFT.h for specification */
boolean CheckerFT_File_isValid(Node_F n) {
    const char* ppath;
    const char* npath;
    const char* rest;
    size_t i;

    /* A null pointer is not a file */
    if(n == NULL) {
        fprintf(stderr, "A file is a null pointer.\n");
        return FALSE;
    }

    /* A null path is not valid */
    if(NodeF_getPath(n) == NULL) {
        fprintf(stderr, "A file's path is a null pointer.\n");
        return FALSE;
    }

    /* A null parent is not valid */
    if(NodeF_getDirectory(n) == NULL) {
        fprintf(stderr, "A file has a null parent directory.\n");
        return FALSE;
    }

    /* Parent's path must be prefix of n's path */
    npath = NodeF_getPath(n);
    ppath = NodeD_getPath(NodeF_getDirectory(n));
    i = strlen(ppath);
    if(strncmp(npath, ppath, i)) {
        fprintf(stderr, "P's path is not a prefix of C's path.\n");
        return FALSE;
    }

    /* n's path after parent's path + '/' must have no further
    '/' characters */
    rest = npath + i;
    rest++;
    if(strstr(rest, "/") != NULL) {
        fprintf(stderr, "C's path has grandchild of P's path.\n");
        return FALSE;
    }

    return TRUE;
}

/* Performs a pre-order traversal of the tree rooted at n. Returns FALSE
if a broken invariant is found and returns TRUE otherwise */
static boolean CheckerFT_treeCheck(Node_D n) {
   size_t i;
   size_t j;

   if(n != NULL) {
        /* Sample check on each non-root node: node must be valid */
        /* If not, pass that failure back up immediately */
        if(!CheckerFT_Dir_isValid(n))
            return FALSE; 
    
        /* Check each file. */
        for(i = 0; i < NodeD_getNumFileChildren(n); i++) {
            Node_F file = NodeD_getFileChild(n, i);

            if(!CheckerFT_File_isValid(file))
                return FALSE;
        }

        /* Check each directory. */
        for(j = 0; j < NodeD_getNumDirChildren(n); j++) {
            Node_D child = NodeD_getDirChild(n, j);

            if(!CheckerFT_Dir_isValid(child))
                return FALSE;
            
            /* if recurring down one subtree results in a failed check
            farther down, passes the failure back up immediately */
            if(!CheckerFT_treeCheck(child))
                return FALSE;
        }
   }
   return TRUE;
}

/* see checkerDT.h for specification */
boolean CheckerFT_isValid(boolean isInit, Node_D root, size_t count) {

   /* if the DT is not initialized, its count should be 0 and
   its root should be NULL */
   if(!isInit) {
      if(count != 0) {
         fprintf(stderr, "Not initialized, but count is not 0.\n");
         return FALSE;
      }
      if(root != NULL) {
         fprintf(stderr, "Not initialized, but root is not NULL.\n");
         return FALSE;
      }
   }

   /* If the count is 0, the root should be NULL. */
   if(count == 0) {
      if(root != NULL) {
         fprintf(stderr, "The count is 0, but the root is not NULL.\n");
         return FALSE;
      }
   }
   /* If the count is positive, the root should not be NULL. */
   if(count >= 1) {
      if (root == NULL) {
         fprintf(stderr, "Count is positive but root is NULL.\n");
         return FALSE;
      }
   }

   /* The root's parent should be NULL when the root is initialized. */
   if(root != NULL) {
      if(NodeD_getParent(root) != NULL) {
         fprintf(stderr, "The root's parent is not NULL.\n");
         return FALSE;
      }
   }

   /* The if the root is NULL, the count should be 0. */
   if(root == NULL) {
      if(count != 0) {
         fprintf(stderr, "The root is null but count is not 0.\n");
         return FALSE;
      }
   }
   
   /* Now checks invariants recursively at each node from the root. */
   return CheckerFT_treeCheck(root);
}
