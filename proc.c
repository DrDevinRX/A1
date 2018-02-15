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

/*
	Compare function for qsort to sort based on burst
*/
static int proc_sort_burst(const void *l, const void *r)
{
	int a = ((proc *)l)->burst;
	int b = ((proc *)r)->burst;

	return (a == b) ? 0 : (a > b) ? 1 : (a < b) ? -1 : 2;
}

/*
	Compare function for qsort to sort based on arrival
*/
static int proc_sort_arrival(const void *l, const void *r)
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
	int c = 0; // current process's index
	bool noOtherProcsRunning = true;
	bool allProcsHaveArrived = false;
	proc *pc = p->procs;

	// Sort procs by arrival time
	qsort(p->procs, p->procCount, sizeof(*(p->procs)), proc_sort_arrival);

	// Start the scheduler
	while (i <= p->time)
	{
		// If the current process has no burst left and the process is marked as not done
		// mark it as not running and done
		if (!pc[c].burst && !pc[c].done)
		{
			pc[c].running = false;
			pc[c].done = true;

			// Print the finish time
			fprintf(fp, "Time %d: %s finished\n", i, pc[c++].name);

			// Mark that no processes are running currently
			noOtherProcsRunning = true;
		}

		// Continue to look for new processes until all processes have arrived.
		if (!allProcsHaveArrived)
		{
			// Check through all of the processes. If the current time matches the arrival time,
			// mark that process as having arrived and print it to the file.
			for (int j = 0; j < p->procCount; j++)
			{
				if (pc[j].arrival == i)
				{
					pc[j].hasArrived = true;
					fprintf(fp, "Time %d: %s arrived\n", i, pc[j].name);
				}
			}

			// We know all processes have arrived if the final process is marked as arrived
			// because we sorted them earlier.
			allProcsHaveArrived = pc[p->procCount - 1].hasArrived;
		}

		// If the current process has arrived, is not running, and no other processes are running,
		// mark the current process are running and print that to the file.
		if (pc[c].hasArrived && !pc[c].running && noOtherProcsRunning)
		{
			noOtherProcsRunning = false;
			pc[c].running = true;
			fprintf(fp, "Time %d: %s selected (burst %d)\n", i, pc[c].name, pc[c].burst);
		}

		// Decrease the current process's burst and increase it's runtime
		if (pc[c].running && pc[c].burst)
		{
			pc[c].burst--;
			pc[c].runtime++;
		}

		// Increase the wait time for all processes that have arrived and are not running.
		for (int j = 0; j < p->procCount; j++)
		{
			if (!pc[j].running && pc[j].hasArrived && !pc[j].done)
				++(pc[j].wait);
		}

		// Check if all processes have finished, if not and no processes are running, the processor is idling,
		// if not, the scheduler is done.
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

	// Print process wait and turnaround time.
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

	// Sort by burst time, not really necessary.
	qsort(p->procs, p->procCount, sizeof(*(p->procs)), proc_sort_burst);
	int t = 0;

	while (t <= p->time)
	{
		// Check if the current process has no burst, is not done, and the scheduler has started.
		// If true, mark the current process as done and not running. Print the finish time.
		if ((!pc[curr].burst) && (!pc[curr].done) && hasStarted)
		{
			pc[curr].running = false;
			pc[curr].done = true;

			fprintf(fp, "Time %d: %s finished\n", t, pc[curr].name);

			procIsRunning = false;
		}

		// While all processes have not arrived, check if a process arrives at that time.
		if (!allProcsHaveArrived(pc, p->procCount))
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

		// Check every process and what process has arrived, is not running, is not done, and has the shortest burst,
		// select that as the current process.
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

		// Decrease the current process's burst and increase the runtime.
		if (pc[curr].running && pc[curr].burst)
		{
			--pc[curr].burst;
			++pc[curr].runtime;
		}

		// Increase wait time for all other processes that have arrived and are not running and are not done.
		for (int i = 0; i < p->procCount; i++)
		{
			if (!pc[i].running && !pc[i].done && pc[i].hasArrived)
				++(pc[i].wait);
		}

		// If the scheduler is not done and no other processes are running and all processes are done,
		// the processor is idle. Otherwise, check if all of the procs are done and if the current time is
		// the time limit, and if so, the scheduler is finished.
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

	// Print process wait and turnaround time.
	for (int i = 0; i < p->procCount; i++)
	{
		fprintf(fp, "%s wait %d turnaround %d\n", pc[i].name, pc[i].wait, (pc[i].runtime + pc[i].wait));
	}
}

static void rr(proc_info *p, FILE *fp)
{
	return;
}

/*
	Check if all processes are done
*/
static bool allProcsAreDone(proc *p, int size)
{
	bool b = true;
	for (int i = 0; i < size; i++)
		b &= p[i].done;
	return b;
}

/*
	Check if all processes have arrived
*/
static bool allProcsHaveArrived(proc *p, int size)
{
	bool b = true;
	for (int i = 0; i < size; i++)
		b &= p[i].hasArrived;
	return b;
}

/*
	Find the process with the shortest burst that has arrived and is not done.
*/
static int shortestCurrBurst(proc *p, int size)
{
	int f = -1; // Final index
	int i = 0; // indexer

	// Check each process until we are out of processes
	while (i + 1 != size)
	{
		int s; // check val
		// Check if the second process is not done and has arrived. Set s to that index.
		if (!p[i + 1].done && p[i + 1].hasArrived)
			s = i + 1;

		// Check if the first process is not done and has arrived.
		if (!p[i].done && p[i].hasArrived)
		{
			// If P1's burst time is less that P2's burst, set s to P1's index.
			if (p[i + 1].hasArrived && !p[i + 1].done)
			{
				if (p[i].burst < p[i + 1].burst)
					s = i;
			}
			else
				s = i;
		}

		// f hasn't been initialized, so set it to the current s value,
		// otherwise check if p[f]'s burst is less than p[s]'s burst.
		// If p[f] < p[s], f remains the same, otherwise f = s.
		if (f < 0)
			f = s;
		else
			f = (p[f].burst < p[s].burst) ? f : s;
		++i;
	}

	return f;
}