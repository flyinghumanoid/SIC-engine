#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include "optab.h"
#include "symtab.h"
#include "error.h"
#define HEXADECIMAL_BASE 16
#define TEXT_RECORD_HEAD_LENGTH 9
#define TEXT_RECORD_MAX_LENGTH 30
#define TEXT_RECORD_LENGTH_POSITION 2
#define TEXT_RECORD_WORD_LENGTH 3
#define LOCCTR                   0      //Location Counter
#define STARTING_ADDRESS         1      //The Starting Address of the Program
#define SYMBOL_ADDRESS           2      //The Address of the Current Symbol in the operand Field
#define FIRST_EXECUTABLE_ADDRESS 3      //The Address of the 1st Executable Instruction
#define PROGRAM_LENGTH           4      //The Total Length of the Assembly Program
#define OBJECT_CODE              5      //The Assembled Object Code
#define MAX_VALUES               6      //Maximum Number of Assembler Variables

void pass1 (void);
void pass2 (void);
//void parseArguments (int argc, char *argv[]);
void parseArguments (char *argv[]);
typedef struct Flags Flags;
struct Flags {
    //Flags set on Assembly error
    unsigned int hasDuplicateLabels     : 1; //Set when Duplicate Labels are Detected
    unsigned int hasInvalidOpcode       : 1; //Set when Invalid opcode (not found in OPTAB) is Detected
    unsigned int hasUndefinedSymbol     : 1; //Set one Undefined Symbols are found in Pass 2 (not present in SYMTAB)
    unsigned int hasError               : 1; //Set when any Error Condition is Detected during Assembly
    //Flags set on Various Assembly Conditions
    unsigned int hasIndexMode           : 1; //Set when the instruction being Processed is in Index Mode
    unsigned int hasImmediateIndexMode  : 1; //Set when the instruction being Processes is in Immediate Index Mode
    unsigned int isExecutableAddressSet : 1; //Set when the First Executable Instruction's Address is got
    unsigned int isNewTextRecord        : 1; //Set when a New Record is requested
};
FILE *source, *destination;                                    //Current Source File Pointer and Destination File Pointer
char* parsedInfo[4];                             //The Parsed information about each assembly line is present here
char sourceName[MAXLEN]= "";                 //The Source File Name
char destinationName[MAXLEN] = "";           //The Destination File Name
char intermediateName[MAXLEN] = "inter.txt"; //The Intermediate File name
OPnode *opcodeNode;                                  //The OPTAB Root
SYMnode *symbolNode;                                //The SYMTAB Root
Flags flags;                                        //Flags that will be used during the assembly
unsigned int aVars[MAX_VALUES];      //Various Assembly time Variables like LOCCTR etc

void assemble(char *argv[])
{
    int i = 0; //Counter/Generic Variables

    //Initilialize string array
    for (i = 0; i < 4; i++)
    {
        parsedInfo[i] = (char *) malloc (MAXLEN * sizeof (char));
    }

    //Parse the Command Line arguments
    parseArguments (argv);

    printf ("Starting Assembly Process...\n\n");

    source = NULL;
    opcodeNode = createOPTabRoot(source,opcodeNode);
    
    //If Memory Allocation for OPTAB failed
    if (opcodeNode == NULL)
    {
      flags.hasError = 1;
      raiseError (0,F_OPTAB_CREATION_FAILED, "");
    }

    //Close file if it is Open
    if (source != NULL)
    {
        fclose (source);
        source = NULL;
    }
    
    source = fopen (sourceName, "r");

    //Check if the file opened Successfully
    if (source == NULL)
    {
        flags.hasError = 1;
        raiseError (0,F_INPUT_FILE_NOT_FOUND, sourceName);
    }

    destination = fopen (intermediateName, "w");

    if (destination == NULL)
    {
        flags.hasError = 1;
        raiseError (0, F_INTERMEDIATE_FILE_CREATE_FAILED, intermediateName);
    }

    pass1(); //Call Pass 1

    //Close any open file pointers
    if (source != NULL)
    {
        fclose (source);
        source = NULL;
    }

    if (destination != NULL)
    {
        fclose (destination);
        destination = NULL;
    }

    if (flags.hasError) {
        raiseError (0, F_GENERIC, "Pass1");
    }
    else {
        printf ("Pass 1 Completed Successfully!!!\n");
    }
    //The Intermediate file is the input for pass 2 
    source = fopen (intermediateName, "r");

    //Check if the file opened Successfully
    if (source == NULL) {
        flags.hasError = 1;
        raiseError (0, F_INPUT_FILE_NOT_FOUND, intermediateName);
    }

    destination = fopen (destinationName, "w"); //The Assembled Object file is output for pass 2

    //Check if the file opened Successfully
    if (destination == NULL) {
        flags.hasError = 1;
        raiseError (0, F_OUTPUT_FILE_CREATE_FAILED, destinationName);
    }

    pass2 (); //Call Pass 2

    //Close the file Handlers
    if (source != NULL) {
        fclose (source);
        source = NULL;
    }

    if (destination != NULL) {
        fclose (destination);
        destination = NULL;
    }
	//Pass 2 non fatal errors, the object code is not generated and the program exits
    if (flags.hasError == 1) {
        raiseError (0, F_GENERIC, "Pass2");
    }
    else {
        printf ("Pass 2 Completed Successfully!!!\n\n");
    }
    if (destination != NULL) {
        fclose (destination);
        destination = NULL;
    }
     //Print Messages According to Error Status
    if (!flags.hasError) {
      printf ("Assembly Successful.\n");
      printf ("Assembled code in \"%s\".\n", destinationName);
      printf ("The Intermediate file is \"inter.txt\"\n");
    }
    else {
        printf ("Assembly Failed :-(.\n");
    }

}

void pass1 (void)
{
    unsigned int daLength = 0;
    unsigned int currAddress = 0;

    //Read first Input line
    getLine (source, parsedInfo);

    if (strcmp (parsedInfo[opcode], "START") == 0) {
        //Save the operand starting address
        sscanf (parsedInfo[operand], "%x",  &aVars[STARTING_ADDRESS]);
        //Initialize LOCCTR to starting Address
        aVars[LOCCTR] = aVars[STARTING_ADDRESS];
        //Write Line to Intermediate File
        fprintf (destination, "%s\t\t%04X\n", parsedInfo[line], aVars[LOCCTR]);
        //Get Next Input Line
        getLine (source, parsedInfo);
    }
    else {
        aVars[LOCCTR] = 0; //Initialize LOCCTR to Zero
    }

    //While opcode not End
    while (strcmp (parsedInfo[opcode], "END") != 0) {
        //If the line is not blank or a comment
        if ((parsedInfo[line][0] != '.') && (parsedInfo[line][0] != '\0')){
            //If there is a SYMBOL in the label Field
            if (parsedInfo[label][0] !=  '\0') {
                //Check if the SYMTAB is initialized
                if (symbolNode == NULL) {
                    symbolNode = createSYMTabRoot(symbolNode, parsedInfo[label], aVars[LOCCTR]);
                    //replace with SYMTAB creation failed
                    if (symbolNode == NULL) {
                      flags.hasError = 1;
                      raiseError (0,F_SYMTAB_CREATION_FAILED,"");
                    }
                }//If SYMTAB not initialized
		else {
                    //Check for Duplicate Label, if not insert the label
                    flags.hasDuplicateLabels = insertSYMTab (symbolNode, parsedInfo[label], aVars[LOCCTR]);

                    //If there are Duplicates set the Error Flag
                    //non fatal error, duplicate labels
                    if (flags.hasDuplicateLabels == 1) {
                        flags.hasError = 1;
                        raiseError (0, E_DUPLICATE_LABEL_FOUND, parsedInfo[label]); //Raise the Error
                        //Output the Error information into the intermediate file
                        fprintf (destination, ".Label %s is Repeated in Code\n", parsedInfo[label]);
                    }//If Duplicates
                }
            }//If there is symbol in label

            //Save the Current Address for Future use...(Currently for storing Addresses into Intermediate File)
            currAddress = aVars[LOCCTR];

            //Search OPTAB for opcode, then increment  LOCCTR 
            if (searchOPTab (opcodeNode, parsedInfo[opcode]) < 0){
                flags.hasInvalidOpcode = 1;
            }
            if (!flags.hasInvalidOpcode) {
                if (flags.isExecutableAddressSet == 0) {
                    flags.isExecutableAddressSet = 1;
                    aVars[FIRST_EXECUTABLE_ADDRESS] = aVars[LOCCTR];
                }
                aVars[LOCCTR] += 3;
            } else if (strcmp (parsedInfo[opcode], "WORD") == 0) {
                aVars[LOCCTR] += 3;
                flags.hasInvalidOpcode = 0;
            } else if (strcmp (parsedInfo[opcode], "RESW") == 0) {
                aVars[LOCCTR] += (atoi(parsedInfo[operand]) * 3);
                flags.hasInvalidOpcode = 0;
            }  else if (strcmp (parsedInfo[opcode], "RESB") == 0) {
                aVars[LOCCTR] += atoi (parsedInfo[operand]);
                flags.hasInvalidOpcode = 0;
	    } else  if (strcmp (parsedInfo[opcode], "BYTE") == 0) {
                daLength = (unsigned int) strlen(parsedInfo[operand]) - 3;
                aVars[LOCCTR] += parsedInfo[operand][0] == 'C' ?  daLength : daLength / 2 ;
                flags.hasInvalidOpcode = 0;
            }
            else  {
                //If opcode not found set error flag
                flags.hasInvalidOpcode = 0;
                flags.hasError = 1;
                raiseError (0, E_OPCODE_NOT_FOUND, parsedInfo[opcode]); //Raise the Error
                //Outpu  the Error information into the intermediate file
                fprintf (destination, ".Operation Code %s doesn't exist\n", parsedInfo[opcode]);
            }//If opcode not found

            //Write Line to Intermediate File
            fprintf (destination, "%s\t\t%04X\n", parsedInfo[line], currAddress);
        }//If not comment

        getLine (source, parsedInfo);
    }//while not end

    //Write Last Line to intermediate file
    fprintf (destination, "%s\t\t%04X\n", parsedInfo[line], aVars[LOCCTR]);
    //Save LOCCTR - Starting Address as Program Length
    aVars[PROGRAM_LENGTH] = aVars[LOCCTR] - aVars[STARTING_ADDRESS];

    DisplaySYMTAB (symbolNode);
}//end Pass 1

void pass2 (void)
{
    unsigned int i = 0; //Generic / Counter Variable
    unsigned int uiRecordLength = 0; //Holds the Current Record Length
    unsigned int uiSymbolAddress = 0; //Holds the Address from SYMTAB of Symbol in OPERAN field
    unsigned int uiOpcodeValue = 0;
    char strObjectRecord[MAXLEN] = "", *ptrObjectRecord; //Object Record and a pointer to it
    char tmpBuff[30] = ""; //A temporary string buffer
    char strByteOrWordOperand[30] = ""; //A string buffer that stores the Operand for Byte or Word, it is used 

    //Initialize the Object Record
    for (i = 0; i < MAXLEN; i++) {
        strObjectRecord[i] = '\0';
    }
    //Initialize the pointer
    ptrObjectRecord = strObjectRecord;

    //Read 1st input line
    getLine (source, parsedInfo);

    //If opcode == "START"
    if ((strcmp (parsedInfo[opcode], "START") == 0)){
        //Convert operand to HEX Value
        i = (unsigned int) strtoul (parsedInfo[operand], NULL, HEXADECIMAL_BASE);
        fprintf (destination, "H%-6s%06X%06X\n", parsedInfo[label], i , aVars[PROGRAM_LENGTH]);
    }//If START

    //Reads from Intermediate File
    getLine (source, parsedInfo);

    //If the 1st opcode is RESB / RESW, Dont generate TEXT Record as of yet
    if (strncmp(parsedInfo[opcode], "RES", 3) != 0) {
        sprintf (strObjectRecord, "T%06X00", aVars[STARTING_ADDRESS]);
    }
//While opcode not End
    while (strcmp (parsedInfo[opcode], "END") != 0) {
        //If the line is not a comment
        if (parsedInfo[line][0] != '.') {
            //Search OPTAB for opcode
            flags.hasInvalidOpcode = uiOpcodeValue = searchOPTab (opcodeNode,  parsedInfo[opcode]);
            if (!flags.hasInvalidOpcode) {
                //If there is a symbol in the operand field
                if (parsedInfo[operand][0] != '\0') {
                    //Check if the Operand is of type Indexed
                    if (strpbrk (parsedInfo[operand], ",") != NULL) {
                        //In SIC, Indexed Address is always with Register X so the part after "," can be ignored
                        strcpy (tmpBuff, strtok (parsedInfo[operand], ","));
                        strcpy (parsedInfo[operand], tmpBuff);

                        //Check the Type of Indexing used
                        if (isdigit (parsedInfo[operand][0])) {
                            flags.hasImmediateIndexMode = 1;
                        }
                        else {
                            flags.hasIndexMode = 1;
                        }
                    }
                     uiSymbolAddress = (unsigned int) searchSYMTab (symbolNode, parsedInfo[operand]);
                    if (uiSymbolAddress == (unsigned int) -1) {
                        flags.hasUndefinedSymbol = 1;
                    }
                    if (!flags.hasUndefinedSymbol || flags.hasImmediateIndexMode) {
                        //Check the type of Addressing mode and Generate the Address
                        if (flags.hasImmediateIndexMode) {
                            //Get the Immediate Value then add 0x8000
                            uiSymbolAddress = (unsigned int) strtoul (parsedInfo[operand], NULL, HEXADECIMAL_BASE);
                            aVars[SYMBOL_ADDRESS] = 0x8000 + uiSymbolAddress;
                        } else if (flags.hasIndexMode) {
                            //Add 0x8000 to Symbol Value
                            aVars[SYMBOL_ADDRESS] = 0x8000 + uiSymbolAddress;
                        }
                        else {
                            //Normal (Direct) Addressing
                            //Store Symbol Value as operand Address
                            aVars[SYMBOL_ADDRESS] = uiSymbolAddress;
                        }
                        /*
                         * Why add 0x8000 you ask ??? Remember the SIC Instruction format :-?
                         * [opcode 8 bits][X 1 bit][operand-ADDRESS 15 bits]
                         * X - The Index Flag in the Instruction has to be set to show indexed Addressing
                         */

                        flags.hasInvalidOpcode = flags.hasUndefinedSymbol = flags.hasIndexMode = flags.hasImmediateIndexMode = 0;
                    }
                    else {
                        aVars[SYMBOL_ADDRESS] = 0x0000;
                        flags.hasError = 1;
                        flags.hasUndefinedSymbol = 0;
                        raiseError (0, E_UNDEFINED_SYMBOL, parsedInfo[operand]); //Raise the Error
                    }

aVars[OBJECT_CODE] = 0x10000 * uiOpcodeValue +  aVars[SYMBOL_ADDRESS];
                }//If symbol
                else {
                    //Case where the opcode has no operand (RSUB in SIC)
                    //Reset Address and Assemble the Code
                    aVars[SYMBOL_ADDRESS] = 0x0000;
                    aVars[OBJECT_CODE] = 0x10000 * uiOpcodeValue +  aVars[SYMBOL_ADDRESS];
                }

                uiRecordLength += TEXT_RECORD_WORD_LENGTH; //Compute Current Record's Length

                //Generate Object Code Only if a New Text Record Request is not Pending
                if (!flags.isNewTextRecord) {
                    sprintf (ptrObjectRecord + strlen (strObjectRecord), "%06X", aVars[OBJECT_CODE]);
                }

            }//If opcode found
            else {
                //Handle BYTE, WORD, RESB, RESW

                //Handle BYTE
                if (strcmp (parsedInfo[opcode], "BYTE") == 0) {

                    //Is the Content Hex ??
                    if (parsedInfo[operand][0] == 'X') {
                        sscanf (parsedInfo[operand], "X'%[^\']", strByteOrWordOperand);
                        //in X' ' Two characters represent 1 Byte
                        uiRecordLength += ((strlen(parsedInfo[operand]) - 3) / 2); //Compute Record Length

                        //Generate Object Code Only if a New Text Record Request is not Pending
                        if (!flags.isNewTextRecord) {
                            //Print String since it is in HEX already                     
                            sprintf (ptrObjectRecord + strlen (strObjectRecord), "%s", strByteOrWordOperand);
                        }
                    }
                    //Is the Content Character ??
                    if (parsedInfo[operand][0] == 'C') {
                        //sscanf (parsedInfo[operand], "C'%[^\']", tempchar);
                        sscanf (parsedInfo[operand], "C'%[^\']", strByteOrWordOperand);
                        //in 'C ' Each character represent 1 Byte
                        uiRecordLength += (strlen(parsedInfo[operand]) - 3); //Compute Record Length

                        //Generate Object Code Only if a New Text Request is not Pending
                        if (!flags.isNewTextRecord) {
                            //Convert the Characters to ASCII (Hex) and store them
                            for (i = 0; i < (unsigned int) strlen (strByteOrWordOperand); i++) {
                                sprintf (ptrObjectRecord + strlen (strObjectRecord), "%01X", (unsigned int) strByteOrWordOperand[i]);
                            }
                        }
                    }
                }

                //Handle WORD
                if(strcmp (parsedInfo[opcode], "WORD") == 0) {
                    aVars[OBJECT_CODE] = atoi (parsedInfo[operand]);
                    uiRecordLength += TEXT_RECORD_WORD_LENGTH;
                    //Generate Object Code Only if a New Text Request is not Pending
                    if (!flags.isNewTextRecord) {
                        sprintf (ptrObjectRecord + strlen (strObjectRecord), "%06X", aVars[OBJECT_CODE]);
                    }
                }

                //Handle RESB
                if (strcmp (parsedInfo[opcode], "RESB") == 0) {
                    uiRecordLength += atoi (parsedInfo[operand]);
                    //Force to break current TEXT Record
                    flags.isNewTextRecord = 1;
                }

                //Handle RESW
                if (strcmp (parsedInfo[opcode], "RESW") == 0) {
                    uiRecordLength += (atoi (parsedInfo[operand]) * TEXT_RECORD_WORD_LENGTH);
                    //Force to break current TEXT Record
                    flags.isNewTextRecord = 1;
                }

            }//Handle BYTE, WORD, RESB, RESW

             //Check if the next record insertion will fit the current TEXT Record OR a New Record is forced
            if ((uiRecordLength + TEXT_RECORD_WORD_LENGTH >  TEXT_RECORD_MAX_LENGTH) || (flags.isNewTextRecord)) {

                //Do not generate Entries for RESB / RESW
                if (strncmp (parsedInfo[opcode], "RES", 3) != 0) {
                    //Finalize the Text Record and Insert the Record Length
                    ptrObjectRecord = ptrObjectRecord + TEXT_RECORD_HEAD_LENGTH - TEXT_RECORD_LENGTH_POSITION;
                    sprintf (tmpBuff, "%02X", (unsigned int) (strlen (strObjectRecord) - TEXT_RECORD_HEAD_LENGTH) / 2);
                    strncpy (ptrObjectRecord, tmpBuff, TEXT_RECORD_LENGTH_POSITION);

                    //Output the Record into the File
                    //If there are RESB/RESW in the beginning of the Assembly,
                    //there is a chance of blank TEXT Record being generated
                    if (strObjectRecord[0] == 'T') {
                        fprintf (destination, "%s\n", strObjectRecord);
                    }

                    //Reset the Record
                    for (i = 0; i < MAXLEN; i++) {
                        strObjectRecord[i] = '\0';
                    }
                    if (flags.isNewTextRecord) {
                        //If it is a RESB
                        if (strcmp (parsedInfo[opcode], "BYTE") == 0) {
                            //Case for Character
                            if (parsedInfo[operand][0] == 'C') {
                                uiRecordLength -= strlen (strByteOrWordOperand);
                            }

                            //Case for Hex
                            if (parsedInfo[operand][0] == 'X') {
                                uiRecordLength -=  (strlen (strByteOrWordOperand) / 2);
                            }
                        }
                        else {
                            //This is the case for RESW
                            uiRecordLength -= TEXT_RECORD_WORD_LENGTH;
                        }
                    }

                    //Prepare a fresh record
                     //Compute Starting  Address of Next Text Record
                    aVars[STARTING_ADDRESS] += uiRecordLength;
                    sprintf (strObjectRecord, "T%06X00", aVars[STARTING_ADDRESS]);
                    ptrObjectRecord = strObjectRecord; //Reset Pointer
                    uiRecordLength = 0; //Reset Values

                   //Generate Object Code for the Currently read opcode,
                    //After the New Text Record Request was made
                    if (flags.isNewTextRecord) {
                        //Code generation is a bit Different for BYTE
                        if (strcmp (parsedInfo[opcode], "BYTE") == 0) {
                            if (parsedInfo[operand][0] == 'X') {
                                //Print String since it is in HEX already
                                sprintf (ptrObjectRecord + TEXT_RECORD_HEAD_LENGTH, "%s", strByteOrWordOperand);
                            }

                            if (parsedInfo[operand][0] == 'C') {
                                //Convert the Characters to ASCII (Hex) and print them          
                                for (i = 0; i < (unsigned int) strlen (strByteOrWordOperand); i++) {
                                    sprintf (ptrObjectRecord + TEXT_RECORD_HEAD_LENGTH, "%01X", (unsigned int) strByteOrWordOperand[i]);
                                }
                            }
                        }
                        else {
                            //Rest of the cases (all other opcodes)
                            sprintf (ptrObjectRecord + TEXT_RECORD_HEAD_LENGTH, "%06X",  aVars[OBJECT_CODE]);
                        }
                        //Set the Record Length
                        uiRecordLength = (unsigned int) (strlen (strObjectRecord) - TEXT_RECORD_HEAD_LENGTH) / 2;
                    }
                    //Reset the New Record Flag
                    flags.isNewTextRecord = 0;
                }
            }
        }//If not comment

        //Reset the Object Code
        aVars[OBJECT_CODE] = 0x000000;

        //Get the next Line from Intermediate File
        getLine (source, parsedInfo);

        //Check for HLT Instruction
        if (strcmp (parsedInfo[opcode], "HLT") == 0) {
            parsedInfo[operand][0] = '\0';
        }
    }//while not END

    //Write the final TEXT Record
    //Finalize the Text Record and Insert the Record Length
    ptrObjectRecord = ptrObjectRecord + TEXT_RECORD_HEAD_LENGTH - TEXT_RECORD_LENGTH_POSITION;
    sprintf (tmpBuff, "%02X", (unsigned int) (strlen (strObjectRecord) - TEXT_RECORD_HEAD_LENGTH) / 2);
    strncpy (ptrObjectRecord, tmpBuff, TEXT_RECORD_LENGTH_POSITION);

    //If there are RESB/RESW in the beginning of the Assembly,
    //there is a chance of blank TEXT Record being generated
    if (strObjectRecord[0] == 'T') {
        //Output the Final Record into the File
        fprintf (destination, "%s\n", strObjectRecord);
    }

    //Write the END Record
    fprintf (destination, "E%06X", aVars[FIRST_EXECUTABLE_ADDRESS]);
}//end Pass 2

void parseArguments (char *argv[])
{
	strcpy(sourceName,argv);
        if (strcmp (destinationName, "") == 0)  {
            strcpy (destinationName, strtok (sourceName, "."));
            strcat (destinationName, ".obj");
        }
	strcpy(sourceName,argv);
}
