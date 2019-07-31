#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct SYMTAB SYMnode;

//Function Prototypes
SYMnode *createSYMTabRoot (SYMnode *SYMroot, char *strSymbol, unsigned int uiAddress);
int insertSYMTab (SYMnode *root, char *strSymbol, unsigned int uiAddress);
int searchSYMTab (SYMnode *root, char *strSymbol);
void DisplaySYMTAB (SYMnode *root);

struct SYMTAB
{
    char symbol[10]; //The symbol found in pass1 LABEL
    unsigned int address; //The address of LABEL
    struct SYMTAB *rptr; //Points to the right sub tree
    struct SYMTAB *lptr; //Points to the left sub tree
};

SYMnode *createSYMTabRoot (SYMnode *SYMroot, char *strLabel, unsigned int uiAddress)
{
    //Malloc the node
    SYMroot = (SYMnode *) malloc (sizeof (SYMnode));
    //Check if the memory is allocated successfully
    if (SYMroot != NULL) {
        //Woo Hoo!!!, We have the root node
        //Copy the respective values into the node
        strcpy (SYMroot->symbol, strLabel);
        SYMroot->address = uiAddress;
        SYMroot->lptr = SYMroot->rptr = NULL;
    }

    //Return the created node
    return SYMroot;
}

int insertSYMTab (SYMnode *root, char *strLabel, unsigned int uiAddress)
{
    static int retval = 0;

    SYMnode *current = root;

    //Does a normal binary tree insert, Has the Duplicate Checking
    if (strcmp(current->symbol, strLabel) != 0) {
        if (strcmp (strLabel, current->symbol) < 0) {
            if (current->lptr == NULL) {
                current->lptr = (SYMnode *)malloc (sizeof (SYMnode));
                current = current->lptr;
                current->lptr = current->rptr = NULL;
                strcpy (current->symbol, strLabel);
                current->address = uiAddress;
                retval = 0;
            }
            else {
                insertSYMTab (current->lptr, strLabel, uiAddress);
            }
        }
        else {
            if (current->rptr == NULL) {
                current->rptr = (SYMnode *)malloc (sizeof (SYMnode));
                current = current->rptr;
                current->lptr = current->rptr = NULL;
                strcpy (current->symbol, strLabel);
                current->address = uiAddress;
                retval = 0;
            }
            else {
                insertSYMTab (current->rptr, strLabel, uiAddress);
            }
        }
    }
    else {
        //Duplicate LABELs Detected
        retval = 1;
    }

    return retval;
}

int searchSYMTab (SYMnode *root, char *strSymbol)
{
    static int retval = -1;//Since the function is recursive the return value
                            //Value need to be maintained between function cal

    SYMnode *current = root;

    //Classic Binary Tree Search in a Binary Tree (Recursive)
    //Need I explain more??? 
    if (strcmp (strSymbol, current->symbol) == 0) {
        //Symbol found
        retval = current->address;
    }
    else {
        if (strcmp (strSymbol, current->symbol) < 0) {
            if (current->lptr == NULL) {
                //No Such Entry
                retval = -1;
            }
            else {
                searchSYMTab (current->lptr, strSymbol);
            }
        }
        else {
            if (current->rptr == NULL) {
                //No Such Entry
                retval = -1;
            }
            else {
                searchSYMTab (current->rptr, strSymbol);
            }
        }
    }

    return retval;
}

void DisplaySYMTAB (SYMnode *root)
{
    //A Typical Inorder Traversal
    if (root->lptr != NULL) {
        DisplaySYMTAB (root->lptr);
    }

    printf ("%s\t%X\n", root->symbol, root->address);

    if (root->rptr != NULL) {
        DisplaySYMTAB (root->rptr);
    }
}
