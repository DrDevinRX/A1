/*
	proc.c
	COP4600 - Assignment 1
*/

#include "proc.h"

// Forward declaration of static functions so that doprocs() doesn't complain
static void rr(proc_info *p, FILE *fp);
static void fcfs(proc_info *p, FILE *fp);
static void sjf(proc_info *p, FILE *fp);
static bool allProcsAreDone(proc *p, int size);
static bool allProcsHaveArrived(proc *p, int size);
static int shortestCurrBurst(proc *p, int size);

// Free the process array
void freeprocs(proc_info *p)
{
	for (int i = 0; i < p->procCount; i++)
		free(p->procs[i].name);
	free(p->procs);
}

void doprocs(proc_info *p)
{
	FILE *fp = fopen("process.out", "w");

	fprintf(fp, "%d process%s\n", p->procCount, (p->procCount > 1) ? "es" : "\0");
	fprintf(
		fp, "Using %s\n",
		(p->type == 0) ? "First Come First Served" : (p->type == 1) ? "Shortest Job First (Pre)" : (p->type == 2) ? "Round-Robin" : "Nothing");

	if (p->quantum > 0)
		fprintf(fp, "Quantum %d\n", p->quantum);

	fprintf(fp, "\n");

	switch (p->type)
	{
	case 0:
		fcfs(p, fp);
		break;
	case 1:
		sjf(p, fp);
		break;
	case 2:
		rr(p, fp);
		break;
	default:
		break;
	}

	fclose(fp);
}

int proc_sort_burst(const void *l, const void *r)
{
	int a = ((proc *)l)->burst;
	int b = ((proc *)r)->burst;

	return (a == b) ? 0 : (a > b) ? 1 : (a < b) ? -1 : 2;
}

int proc_sort_arrival(const void *l, const void *r)
{
	int a = ((proc *)l)->arrival;
	int b = ((proc *)r)->arrival;

	return (a == b) ? 0 : (a > b) ? 1 : (a < b) ? -1 : 2;
}

/*
	First-Come First-Served - Processes are run in the order they arrive
*/
static void fcfs(proc_info *p, FILE *fp)
{
	int i = 0;
	int c = 0;
	bool noOtherProcsRunning = true;
	bool allProcsHaveArrived = false;
	proc *pc = p->procs;

	qsort(p->procs, p->procCount, sizeof(*(p->procs)), proc_sort_arrival);

	while (i <= p->time)
	{

		if (!pc[c].burst && !pc[c].done)
		{
			pc[c].running = false;
			pc[c].done = true;

			fprintf(fp, "Time %d: %s finished\n", i, pc[c++].name);

			noOtherProcsRunning = true;
		}

		if (!allProcsHaveArrived)
		{
			for (int j = 0; j < p->procCount; j++)
			{
				if (pc[j].arrival == i)
				{
					pc[j].hasArrived = true;
					fprintf(fp, "Time %d: %s arrived\n", i, pc[j].name);
				}
			}

			allProcsHaveArrived = pc[p->procCount - 1].hasArrived;
		}

		if (pc[c].hasArrived && !pc[c].running && noOtherProcsRunning)
		{
			noOtherProcsRunning = false;
			pc[c].running = true;
			fprintf(fp, "Time %d: %s selected (burst %d)\n", i, pc[c].name, pc[c].burst);
		}

		if (pc[c].running && pc[c].burst)
		{
			pc[c].burst--;
			pc[c].runtime++;
		}

		for (int j = 0; j < p->procCount; j++)
		{
			if (!pc[j].running && pc[j].hasArrived && !pc[j].done)
				++(pc[j].wait);
		}

		if (!pc[p->procCount - 1].done && noOtherProcsRunning)
		{
			fprintf(fp, "Time %d: IDLE\n", i);
		}
		else if (pc[p->procCount - 1].done)
		{
			fprintf(fp, "Finished at time %d\n\n", i);
		}

		i++;
	}

	for (i = 0; i < p->procCount; i++)
	{
		fprintf(fp, "%s wait %d turnaround %d\n", pc[i].name, pc[i].wait, (pc[i].runtime + pc[i].wait));
	}
}

/*
	Shortest Job First (Pre)
*/
static void sjf(proc_info *p, FILE *fp)
{
	proc *pc = p->procs;
	bool procIsRunning = false;
	int curr = 0;
	bool hasStarted = false;

	qsort(p->procs, p->procCount, sizeof(*(p->procs)), proc_sort_burst);
	int t = 0;
	proc currProc = pc[curr];

	while (t <= p->time)
	{
		if ((!pc[curr].burst) && (!pc[curr].done) && hasStarted)
		{
			pc[curr].running = false;
			pc[curr].done = true;

			fprintf(fp, "Time %d: %s finished\n", t, pc[curr].name);

			procIsRunning = false;
		}

		if (!allProcsAreDone(pc, p->procCount))
		{
			for (int i = 0; i < p->procCount; i++)
			{
				if (pc[i].arrival == t && !pc[i].hasArrived)
				{
					pc[i].hasArrived = true;
					hasStarted = true;
					fprintf(fp, "Time %d: %s arrived\n", t, pc[i].name);
				}
			}
		}

		for (int i = 0; i < p->procCount; i++)
		{
			if (pc[i].hasArrived && !pc[i].running && !pc[i].done && i == shortestCurrBurst(pc, p->procCount))
			{
				pc[curr].running = false;
				pc[i].running = true;
				procIsRunning = true;
				curr = i;
				fprintf(fp, "Time %d: %s selected (burst %d)\n", t, pc[curr].name, pc[curr].burst);
				break;
			}
		}

		if (pc[curr].running && pc[curr].burst)
		{
			--pc[curr].burst;
			++pc[curr].runtime;
		}

		for (int i = 0; i < p->procCount; i++)
		{
			if (!pc[i].running && !pc[i].done && pc[i].hasArrived)
				++(pc[i].wait);
		}

		if (allProcsAreDone(pc, p->procCount) && !procIsRunning && t != p->time)
		{
			fprintf(fp, "Time %d: IDLE\n", t);
		}
		else if (allProcsAreDone(pc, p->procCount) && !procIsRunning)
		{
			fprintf(fp, "Finished at time %d\n\n", t);
		}
		t++;
	}

	for (int i = 0; i < p->procCount; i++)
	{
		fprintf(fp, "%s wait %d turnaround %d\n", pc[i].name, pc[i].wait, (pc[i].runtime + pc[i].wait));
	}
}

static void rr(proc_info *p, FILE *fp)
{
	return;
}

static bool allProcsAreDone(proc *p, int size)
{
	bool b = true;
	for (int i = 0; i < size; i++)
		b &= p[i].done;
	return b;
}

static bool allProcsHaveArrived(proc *p, int size)
{
	bool b = true;
	for (int i = 0; i < size; i++)
		b &= p[i].hasArrived;
	return b;
}

static int shortestCurrBurst(proc *p, int size)
{
	int f = -1;
	int i = 0;

	while (i + 1 != size)
	{
		int s;
		if (!p[i + 1].done && p[i + 1].hasArrived)
			s = i + 1;

		if (!p[i].done && p[i].hasArrived)
		{
			if (p[i + 1].hasArrived && !p[i + 1].done)
			{
				if (p[i].burst < p[i + 1].burst)
					s = i;
			}
			else
				s = i;
		}

		if (f < 0)
			f = s;
		else
			f = (p[f].burst < p[s].burst) ? f : s;
		++i;
	}

	return f;
}