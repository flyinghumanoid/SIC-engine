#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "getline.h"

typedef struct OPTAB OPnode;

//Function Prototypes
OPnode *createOPTabRoot (FILE *fpSource, OPnode *OProot);
int searchOPTab (OPnode *root, char *strOpcode);
int insertOPTab (OPnode *root);
void DisplayOPTAB (OPnode *root);
int mnemonic = 1,op = 2, var = 2;

#define TOTAL_INSTRUCTIONS 25

//Structure that Holds the OPTAB (One node of the Binary Tree)
struct OPTAB
{
    char opcode[10]; //The opcode mnemonic
    unsigned int value; //The value of mnemonic
    struct OPTAB *rptr; //Points to the right sub tree
    struct OPTAB *lptr; //Points to the left sub tree
};

//Internal OPTAB, used if External OPTAB not supplied
const char OPCodes[TOTAL_INSTRUCTIONS][3][MAXLEN] = {
    {"J\t3C\0","J\0", "3C\0" },
    {"LDA\t00\0","LDA\0", "00\0" },
    {"SUB\t1C\0","SUB\0", "1C\0" },
    {"JLT\t38\0","JLT\0", "38\0" },
    {"STX\t10\0","STX\0", "10\0" },
    {"LDCH\t50\0","LDCH\0", "50\0" },
    {"STCH\t54\0","STCH\0", "54\0" },
    {"TD\tE0\0","TD\0", "E0\0" },
    {"RSUB\t4C\0","RSUB\0", "4C\0" },
    {"STL\t14\0","STL\0", "14\0" },
    {"LDX\t04\0","LDX\0", "04\0" },
    {"STA\t0C\0","STA\0", "0C\0" },
    {"ADD\t18\0","ADD\0", "18\0" },
    {"JSUB\t48\0","JSUB\0", "48\0" },
    {"COMP\t28\0","COMP\0", "28\0" },
    {"JEQ\t30\0","JEQ\0", "30\0" },
    {"LDL\t08\0","LDL\0", "08\0" },
    {"RD\tD8\0","RD\0", "D8\0" },
    {"AND\t58\0","AND\0", "58\0"},
    {"OR\t44\0","OR\0", "44\0"}, 
    {"TIX\t2C\0","TIX\0", "2C\0" },
    {"WD\tDC\0","WD\0", "DC\0" },
    {"MUL\t20\0","MUL\0", "20\0" },
    {"JGT\t34\0","JGT\0", "34\0" },
    {"DIV\t24\0","DIV\0", "24\0" },
};

char* strParsedInfo[4]; //Stores the Parsed Info from each line

OPnode *createOPTabRoot (FILE *fpSource, OPnode *OProot)
{
    int i = 0; //Counter Variable

    //Initialize the string Array which is gonna store the Parsed Information
    for (i = 0; i < 4; i++)
    {
        strParsedInfo[i] = (char *) malloc (100 * sizeof (char));
    }

    //Check to see whether to load the internal or external OPTAB
    if (fpSource == NULL)
    {
        //Loop until all the Instructions are loaded
        for (i = 0; i < TOTAL_INSTRUCTIONS; i++)
	{
            strcpy (strParsedInfo[line], OPCodes[i][line]); //Copy the Line
            strcpy (strParsedInfo[op], OPCodes[i][mnemonic]); //Copy the mnemonic
            strcpy (strParsedInfo[operand], OPCodes[i][var]); //Copy the value

            //Actually if you ask me why this NULL Check exists,
            //It was done for Saftey checks in the previous Assembler builds
            //As of now the modules are Safe, But the Check Exists as an extra 
            //Barricade, just in case !!!
            //Check to see if the root is NULL (first insert)       
            if (OProot == NULL)
	    {
                //Malloc the root Root
                OProot = (OPnode *) malloc (sizeof (OPnode));
                if (OProot == NULL)
                {
                    printf ("Assembly Error: OPTAB Creation Failed, Insufficient Memory\n");
                    exit (EXIT_FAILURE);
                }
                //Woo Hoo!!!, We have the root node
                //Copy the respective values
                strcpy (OProot->opcode,  strParsedInfo[op]);
                sscanf (strParsedInfo[operand], "%x", &OProot->value);
                OProot->lptr = OProot->rptr = NULL;
            }
            else
            {
                //Else do a Normal Insert
                if (insertOPTab (OProot) == 1)
                {
                    printf ("Assembly Error: Duplicate mnemonic Values in Table\n");
                    exit (EXIT_FAILURE);
                }
	    }
        }
    }
    else
    {
        //This is done if the Input is from a file
        //Except for the getLine, rest of the Logic remains the same
        while (!feof(fpSource))
        {
            //Get the line calling our parser
            getLine (fpSource, strParsedInfo);
            if (OProot == NULL)
            {
                OProot = (OPnode *) malloc (sizeof (OPnode));
                if (OProot == NULL)
                {
                    printf ("Assembly Error: OPTAB Creation Failed, Insufficient Memory\n");
                    exit (EXIT_FAILURE);
                }

                strcpy (OProot->opcode,  strParsedInfo[op]);
                sscanf (strParsedInfo[operand], "%x", &OProot->value);
                OProot->lptr = OProot->rptr = NULL;
            }
            else
            {
                if (insertOPTab (OProot) == 1)
                {
                    printf ("Assembly Error: Duplicate mnemonic Values in Table\n");
                    exit (EXIT_FAILURE);
                }
            }
        }
    }
    //DisplayOPTAB (OProot);
    //exit(0);
    //Return the root node
    return OProot;
}

int searchOPTab (OPnode *root, char *strOpcode)
{
    static int retval = -1; //Since the function is recursive the return value
                            //Value need to be maintained between function calls

    OPnode *current = root;

    //Classic Binary Tree Search in a Binary Tree (Recursive)
    //Need I explain more??? 
    //Hint: If you dont get it, go look into Data Stuctures :-p
    if (strcmp (strOpcode, current->opcode) == 0) {
        //Opcode Found
        retval = current->value;
    }
    else {
        if (strcmp (strOpcode, current->opcode) < 0) {
            if (current->lptr == NULL) {
                //No Such Entry
                retval = -1;
            }
            else {
                searchOPTab (current->lptr, strOpcode);
            }
        }
        else {
            if (current->rptr == NULL) {
                //No Such Entry
                retval = -1;
            }
            else {
                searchOPTab (current->rptr, strOpcode);
            }
        }
    }

    return retval;
}

int insertOPTab (OPnode *root)
{
    static int retval = 0;

    OPnode *current = root;

    //Does a normal binary tree insert, Has the Duplicate Checking
    if (strcmp(current->opcode, strParsedInfo[op]) != 0)
    {
        if (strcmp (strParsedInfo[op], current->opcode) < 0)
        {
            if (current->lptr == NULL)
            {
                current->lptr = (OPnode *)malloc (sizeof (OPnode));
                current = current->lptr;
                current->lptr = current->rptr = NULL;
                strcpy (current->opcode,strParsedInfo[op]);
                sscanf (strParsedInfo[operand], "%x", &current->value);
                retval = 0;
            }
            else
            {
                insertOPTab (current->lptr);
            }
        }
        else
        {
            if (current->rptr == NULL)
            {
                current->rptr = (OPnode *)malloc (sizeof (OPnode));
                current = current->rptr;
                current->lptr = current->rptr = NULL;
                strcpy (current->opcode, strParsedInfo[op]);
                sscanf (strParsedInfo[operand], "%x", &current->value);
                retval = 0;
            }
            else
            {
                insertOPTab (current->rptr);
            }
        }
    }
    else
    {
        //Duplicates in mnemonic Detected
        retval = 1;
    }

    return retval;
}

void DisplayOPTAB (OPnode *root)
{
    //A Typical Inorder Traversal
    if (root->lptr != NULL)
    {
        DisplayOPTAB (root->lptr);
    }

    printf ("%s\t", root->opcode);
    printf ("%02X\n", root->value);

    if (root->rptr != NULL)
    {
        DisplayOPTAB (root->rptr);
    }
}

