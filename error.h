#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define F_OPTAB_FILE_NOT_FOUND 0
#define F_INPUT_FILE_NOT_FOUND 1
#define F_INTERMEDIATE_FILE_CREATE_FAILED 2
#define F_INTERMEDIATE_FILE_NOT_FOUND 3
#define F_OUTPUT_FILE_CREATE_FAILED 4
#define F_INPUT_FILE_MISSING 5
#define F_OPTAB_CREATION_FAILED 6
#define F_SYMTAB_CREATION_FAILED 7
#define F_NO_FILE_NAME 8
#define F_BOOT_AND_LOAD_ASSEMBLE 9
#define F_BOOTSTRAP_LENGTH_EXCEEDED 10
#define F_DEVICE_NAME_MISSING 11
#define F_GENERIC 12

#define E_DUPLICATE_LABEL_FOUND 0
#define E_OPCODE_NOT_FOUND 1
#define E_UNDEFINED_SYMBOL 2

/*#define NO_OPCODE_FOUND 
#define NO_SYMBOL_FOUND 
*/
#define MAXLEN 100
#define MAX_FATAL 20
#define MAX_ERROR 20
#define MAX_WARN 20

void raiseError(unsigned int iErrorLevel, unsigned int iErrorType, char strMessage[])
{
  char strFatalMessage[MAX_FATAL][MAXLEN];
  char strErrorMessage[MAX_ERROR][MAXLEN];
  //char strWarningMessage[MAX_WARN][MAXLEN]; //As of now no Warning Messages exists

  //Various FATAL Messages
  strcpy (strFatalMessage[F_OPTAB_FILE_NOT_FOUND], "Could not Open OPTAB");
  strcpy (strFatalMessage[F_INPUT_FILE_NOT_FOUND], "Could not Open Input Fille");
  strcpy (strFatalMessage[F_INTERMEDIATE_FILE_CREATE_FAILED], "Could not Create/Open Intermediate File");
  strcpy (strFatalMessage[F_OUTPUT_FILE_CREATE_FAILED], "Could not Create/Open Output File");
  strcpy (strFatalMessage[F_INPUT_FILE_MISSING], "Source File not supplied");
  strcpy (strFatalMessage[F_OPTAB_CREATION_FAILED], "Memory Allocation for OPTAB Failed");
  strcpy (strFatalMessage[F_SYMTAB_CREATION_FAILED], "Memory Allocation for SYMTAB Failed");
  strcpy (strFatalMessage[F_NO_FILE_NAME], "Output file name missing for -o");
  strcpy (strFatalMessage[F_BOOT_AND_LOAD_ASSEMBLE], "Cannot do both Bootstrap and Loader Assembly in one shot");
  strcpy (strFatalMessage[F_BOOTSTRAP_LENGTH_EXCEEDED], "Bootstrap Creation failed, Expected Bootstrap length Exceeded for");
  strcpy (strFatalMessage[F_DEVICE_NAME_MISSING], "Output Device Missing for -d");
  strcpy (strFatalMessage[F_GENERIC], "Stopping Assembly Due to Error in ");

  //Various ERROR Messages
  strcpy (strErrorMessage[E_DUPLICATE_LABEL_FOUND], "Duplicate LABEL present in Code");
  strcpy (strErrorMessage[E_OPCODE_NOT_FOUND], "Invalid Operation Code (OPCODE) in Code");
  strcpy (strErrorMessage[E_UNDEFINED_SYMBOL], "Undefeind Symbol present in the Code");


  //Print the Messages to sterr
  //Detects the Error Level and then Prints the Corresponding Message
  switch (iErrorLevel) {
  case 0:
      fprintf(stderr, "Assembly Error (FATAL) : ");
    switch (iErrorType) {
    case F_OPTAB_FILE_NOT_FOUND:
    case F_INPUT_FILE_NOT_FOUND:
    case F_INTERMEDIATE_FILE_CREATE_FAILED:
    case F_OUTPUT_FILE_CREATE_FAILED:
        fprintf (stderr, "%s \"%s\"\n",strFatalMessage[iErrorType], strMessage);
        break;
    
    case F_INPUT_FILE_MISSING:
    case F_OPTAB_CREATION_FAILED:
    case F_SYMTAB_CREATION_FAILED:
    case F_NO_FILE_NAME:
    case F_BOOT_AND_LOAD_ASSEMBLE:
        fprintf (stderr, "%s\n",strFatalMessage[iErrorType]);
        break;

    case F_BOOTSTRAP_LENGTH_EXCEEDED:
        fprintf (stderr, "%s \"%s\"\n",strFatalMessage[iErrorType], strMessage);
        break;

    case F_DEVICE_NAME_MISSING:
        fprintf (stderr, "%s\n",strFatalMessage[iErrorType]);
        break;

    case F_GENERIC:
        fprintf (stderr, "%s \"%s\"\n", strFatalMessage[iErrorType], strMessage);
        break;

    default:
        break;
    }
    exit (EXIT_FAILURE); //Exit after FATAL Messages
    //break;

    case 1:
      fprintf(stderr, "Assembly Error (ERROR) : ");
      switch (iErrorType) {
      case E_DUPLICATE_LABEL_FOUND:
          fprintf (stderr, "%s\n \"%s\" is repeated in the Code\n", strErrorMessage[iErrorType], strMessage);
          break;

      case E_OPCODE_NOT_FOUND:
          fprintf (stderr, "%s\n \"%s\" Does not exist in OPTAB\n", strErrorMessage[iErrorType], strMessage);
          break;

      case E_UNDEFINED_SYMBOL:
          fprintf (stderr, "%s\n \"%s\" Does not exist in SYMTAB\n", strErrorMessage[iErrorType], strMessage);
          break;

      default:
          break;
      }
    break;

  case 2:
      //Not used as of now
      break;

  default:
      break;
  }
}
