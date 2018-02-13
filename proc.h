/*
	Devin Goldstein
	proc.h
	COP4600 - Assignment 1
*/

#ifndef _PROC_H
#define _PROC_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct proc_t
{
	char *name;
	int arrival;
	int burst;
	int runtime;
	int wait;
	bool hasArrived;
	bool running;
	bool done;
} proc;

typedef struct proc_info_t
{
	int type;
	int time;
	int quantum;
	int procCount;
	proc *procs;
} proc_info;

void freeprocs(proc_info *p);
void doprocs(proc_info *p);

#endif
