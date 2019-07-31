#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include "asm.h"
#include "sicengine.c"

void processExe(int x,char ch[][64]);
void help();
unsigned long STARTADD;

int main()
{
	char sparsed[4][64]; //Space Parsed Argument Variable, First Array holds Space Characters, Second Holds Commands, Third holds first parameter and Fourth holds second parameter
	//char primArgv[128];
	const char *directives[10] = {"load\0","execute\0","debug\0","dump\0","help\0","assemble\0","dir\0","directory\0","clear\0","exit\0"};//Array of Arrays
	bool exitstatus = false; //Set to false so that exitstatus is initialized
	SICInit();
	int increBuck=0;
	int length = 0;
	int end[4] = {0,0,0,0};
	char argv[128]; //Holds user input
	bool match; //Verifies match in array of arrays
	printf("Welcome to SIC Phase 4!\n");
	do
	{
		do
		{
			printf(">");
			fgets(argv,128,stdin);
		}while(argv[0] == '\n'); //Continues searching for input while user presses enter
		bool match = false;
		sparsed[0][0]=sparsed[1][0]=sparsed[2][0]=sparsed[3][0] = '\0'; // reset all the parsed strings
		int i = 0;
		int j = 0;
		increBuck = 0;
		end[0] = end [1] = end [2] = end[3] = 0;
		length = 0; //initialize variable outside of for condition
		for(; i < 128; i++) //iterate through string
		{
			if(argv[i] == '\n') //locate carriage return and
			{
				argv[i] = '\0'; // replace with NULL char
				length = i;
				break;
			}
		}
		for(i=0; i< length; i++) // loop through the length of the arg variable
		{
			if((argv[i] == ' ')) //if starting with zeros, skip
			{
				while(argv[i]==' ')
				{
					i++;
				}
			}
			if((argv[i]=='\0')) //stop at \0
			{
				break;
			}
			while((argv[i]!=' ')&&(argv[i]!='\0')) //keep going until space is found or null
			{
				sparsed[increBuck][j] = argv[i]; //set parsed arguments with argv by incrementing both at the same time
				j++; //increment independetly because i might be bigger
				i++;
			}
			end[increBuck] = j; //when word is stored record the end point
			increBuck++; //increment bucket to be used in next param
			j=0;//reset j to be used again by loop
		}
		for(i=0;i<=increBuck;i++)
		{
			sparsed[i][end[i]] = '\0'; //fill null character in newly created strings from argv
		}
		for(j=0; j <= 10; j++) //iterate through directives
		{
			if(j == 10) //last pass
			{
				if(match == false) //if still false, then string not recognized
				{
					processEx(10,sparsed,increBuck); //sent down to default case
				}
			}
			else
			{
				for(i=0;i < 128;i++) //iterate through characters
				{
					if(sparsed[0][i] == directives[j][i]) //compare
					{
						if(sparsed[0][i] == '\0') //until null is reached
						{
							if(j == 9)
							{
								exitstatus = true; //if matched with exit, then set exitstatus to true
								match = true;
								break;
							}
							else
							{
								processEx(j,sparsed,increBuck); //if matched with something else, run respective process
								match = true;
								break;
							}
						}
					}
					else
					{
						break; //not matching,skip
					}
				}

			}
		}
	}while(!exitstatus);
}

void processEx(int x, char ch[][64],int param)
{
	switch(x)
	{
		case 0:
		{
			if(param != 2)
			{
				printf("Load command takes 1 parameter!!\n");
				break;
			}
			else
			{
				int len = strlen(ch[1]);
				char * last = &ch[1][len-4];
				if(!strcmp(last,".obj"))
				{
					FILE * fpSource = fopen(ch[1],"r");
					FILE * fpDestination = fopen("outdat.txt","w");
					char strObjectRecord[MAXLEN]; //Object Record from assembly file
    					char strStrippedObjectRecord[MAXLEN]; //Stripped Record for Bootstrap
					unsigned long ADDRESS;
					unsigned char * BYTE[1];
   					//Counter Variables
					unsigned int i = 0;
   			        	unsigned int j = 0;
					unsigned int q;
    					//Initialize the string
    					for (i = 0; i < MAXLEN; i++)
	 				{
        					strStrippedObjectRecord[i] = '\0';
    					}

    					//Read till the EOF of Object File
    					while (!feof(fpSource))
				 	{
        					i = 1;

        					//Get the Object code from Source file
        					fgets (strObjectRecord, MAXLEN, fpSource);

        					//Process only TEXT Records
        					if (strObjectRecord[0] == 'T')
 						{
            						while(i < 7)
							{
								strStrippedObjectRecord[j++] = strObjectRecord[i++];
							}
							ADDRESS = (unsigned long)strtoul(strStrippedObjectRecord,NULL,16);
							i++;i++;

							//Check for the End of current TEXT Record
            						while ((strObjectRecord[i] != '\0') && (strObjectRecord[i] != '\n'))
	 						{
                						//Put the Assembled code (Excluding The Record Head) to the Stripped Record
               		 					BYTE[0] = strStrippedObjectRecord[j++] = strObjectRecord[i++];
								PutMem(ADDRESS,&BYTE,0);
								ADDRESS++;
            						}
        					}
						//Put the End marker
						i = 1;j=0;
						if(strObjectRecord[0] == 'E')
						{
							while(strObjectRecord[i] != '\0')
							{
								strStrippedObjectRecord[j++] = strObjectRecord[i++];
							}
							STARTADD = (unsigned long)strtoul(strStrippedObjectRecord,NULL,16);
							printf("Start address is %x\n",STARTADD);

						}
        					//Output the line, it is not empty
        					//Initialize the string
        					for (i = 0; i < MAXLEN; i++)
			 			{
            						strStrippedObjectRecord[i] = '\0';
        					}
        					//Reset j
        					j = 0;
    					}
					if(fpDestination == NULL)
					{
						printf("Outdat.txt creation failed...\n");
					}
					else
					{
						printf("Object code success...\n");
						fclose(fpDestination);
						fclose(fpSource);
					}
				}
				else
				{
					printf("Object File not Found\n");
				}
				break;
			}
		}
		case 1:
		{
				SICRun(STARTADD,0);
				break;
		}
		case 2:
		{
			if(param != 2)
			{
				printf("Debug command takes 1 parameter!!\n");
				break;
			}
			else
			{


				break;
			}
		}
		case 3:
		{
			if(param != 3)
			{
				printf("Dump command takes 2 parameters!!\n");
				break;
			}
			else
			{
				unsigned char * VAL[2];
				unsigned long FIRSTADDRESS = strtoul(ch[1],NULL,16);
				unsigned long SECONDADDRESS = strtoul(ch[2],NULL,16);
				unsigned long i;
			        for (i = FIRSTADDRESS; i < SECONDADDRESS;i++)
				{
   	         			if(i%16==0)
					{
        	       				printf("\n%04X ",i);
      		      			}
    	   	     			else
				 	{
   	             				if(i%4==0)
						{
       	             					printf(" ");
	                			}
        	    			}
					GetMem(i,VAL,0);
	            			printf("%s",VAL);
       		 		}
      		  		printf ("\n");

			}
			break;
		}
		case 4:
		{
			//help function
			help();
			break;
		}
		case 5:
		{
			if(param != 2)
			{
				printf("Assemble command takes 1 parameter!!\n");
				break;
			}
			else
			{
				assemble(ch[1]);
				break;
			}
		}
		case 6:
		case 7:
		{
			//directory
			system("ls");
			break;
		}
		case 8:
		{
			system("clear");
			break;
		}
		default:
		{
			errorThrow(ch[0]);
			break;
		}
	}
}

void help()
{
	printf("\tload (filename) : will call the load function to load the specified file.\n");
	printf("\texecute : will call the computer simulation program to execute the program that was previously loaded in memory\n");
	printf("\tdebug : will allow you to execute in debug mode.\n");
	printf("\tdump (start) (end) : will call the dump funtion, passing the values of start and end. If start and end are not given, ask for them. Start and end will be hexadecimal values.\n");
	printf("\thelp : prints out this table\n");
	printf("\tassemble filename : will assemble an SIC assembly language program into a load module and store it in a file\n");
	printf("\tdirectory or dir: will list the files stored in the current directory\n");
	printf("\tclear: will clear the terminal\n");
	printf("\texit: will exit the simulator\n");
}
void errorThrow(char ch[])
{
	printf("%s: not recognized, type help for supported functions\n",ch);
}
