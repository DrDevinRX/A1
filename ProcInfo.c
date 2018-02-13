/*
	Devin Goldstein
	ProcInfo.c
	COP4600 - Assignment 1
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "proc.h"

int main(void)
{
	// Init a proccess info struct
	proc_info p = {.quantum = 0};
	// file pointer for open
	FILE *fp;

	// Attempt to open the file and if it fails, terminate the program.
	if (!(fp = fopen("set3_process.in", "r")))
	{
		printf("Could not open file!\n");
		return 1;
	}

	// Buffers for fgets and sscanf
	char get[255];
	char *b = calloc(25, sizeof(char));
	char *c = calloc(25, sizeof(char));

	// Counters
	int lcount = 0;
	int i = 0;

	// Read the process file and store all info into the proc_info struct
	while (strcmp(fgets(get, 255, fp), "end\n"))
	{
		// First four lines directly relate to the proc_info struct
		if (++lcount <= 4)
		{
			sscanf(get, "%s %s", b, c);
			if (!strcmp(b, "processcount"))
			{
				// Get amount of processes and allocate memory for the proc pointer.
				p.procCount = strtol(c, NULL, 10);
				p.procs = malloc(sizeof(proc) * p.procCount);

				// Init the proc structs
				for (int j = 0; j < p.procCount; j++)
				{
					p.procs[j].done = false;
					p.procs[j].running = false;
					p.procs[j].hasArrived = false;
					p.procs[j].wait = 0;
					p.procs[j].runtime = 0;
				}
			}

			// Runtime of process block
			if (!strcmp(b, "runfor"))
				p.time = strtol(c, NULL, 10);

			// Which processing strategy to use
			if (!strcmp(b, "use"))
				p.type =
					(!strcmp(c, "fcfs")) ? 0 : (!strcmp(c, "sjf")) ? 1 : (!strcmp(c, "rr")) ? 2 : -1;

			// Quantum if applicable
			if (!strcmp(b, "quantum"))
				p.quantum = strtol(c, NULL, 10);
		}

		// Lines that directly relate to the individual process
		else
		{
			int j = 1;

			char *split = strtok(get, " ");
			while (split)
			{
				j++;
				split = strtok(NULL, " ");

				if (j == 3)
				{
					p.procs[i].name = malloc(sizeof(char) * (strlen(split) + 1));
					strcpy(p.procs[i].name, split);
				}

				if (j == 5)
					p.procs[i].arrival = strtol(split, NULL, 10);

				if (j == 7)
				{
					p.procs[i].burst = strtol(split, NULL, 10);
					break;
				}
			}

			i++;
		}
	}

	//	Free our pointers, we won't need them anymore
	free(b);
	free(c);
	fclose(fp);

	//	Process the processes
	doprocs(&p);
	//	Free our process pointer
	freeprocs(&p);

	return 0;
}
