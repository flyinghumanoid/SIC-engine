#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define MAXLEN 100
int line = 0,label = 1,opcode = 2,operand = 3;

void getLine (FILE * source, char* parsedInfo[])
{
    int length = 0;

    char buff[100];

    fgets (buff, 100, source);

    length = strlen (buff);
    if (buff[length - 1] == '\n') {
        buff[length - 1] = '\0';
    }
    strcpy (parsedInfo[line], buff);

    parsedInfo[label][0] = parsedInfo[opcode][0] = parsedInfo[operand][0] = '\0';
    if (parsedInfo[line][0] != '.') {
        if (!isspace(parsedInfo[line][0])) 
        {
            sscanf (parsedInfo[line], "%s%s%s", parsedInfo[label], parsedInfo[opcode], parsedInfo[operand]);
        }
        else
        {
            sscanf (parsedInfo[line], "%s%s", parsedInfo[opcode], parsedInfo[operand]);
        }
        if ((strcmp (parsedInfo[opcode], "RSUB") == 0) && (strcmp (parsedInfo[operand], "4C") != 0)) {
            parsedInfo[operand][0] = '\0';
        }
    }
}
