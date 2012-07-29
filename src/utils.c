#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>

#include "common.h"
#include "elf_abi.h"

void runelf(void)
{
	extern unsigned char elfldr_start, elfldr_end;
	void *elfldr = (void*)0x9b000000;
	((int*)elfldr)[-1] = 0;
	extern void run();
	memcpy(elfldr, &elfldr_start, &elfldr_end - &elfldr_start);
	memicbi(elfldr, &elfldr_end - &elfldr_start);
	
	//printf("elfldr %p, run %p, elfldr_start %p\n", elfldr, run, &elfldr_start);
	void (*elfldr_run)() = elfldr + (((unsigned char*)run) - &elfldr_start);
	//printf("relocated elfldr_run %p\n\n", elfldr_run);

	//printf("waiting for other threads...\n");
	int i;
	for (i = 1; i < 6; ++i)
		while (xenon_run_thread_task(i, 0, elfldr_run));
	mdelay(100);
	//printf("launching...\n");

	elfldr_run();
	//printf("returned?!\n");
}

int exec( char *filename )
{
	const char *argument = filename;

	//printf("loading.. %s\n", filename);
	//sprintf(progress, "Loading %s", filename);
	
	void *target = (void*)0x9C000000;
	int f = open(filename, O_RDONLY);
	if (f < 0)
	{
		strcpy(progress, "Load failed.");
		return 0;
		//goto a;
	}
	
	struct stat s;
	fstat(f, &s);
	
	int rem = s.st_size;
	
	if (rem < 1024)
	{
		strcpy(progress, "Invalid binary");
		//goto a;
		return 0;
	}
	
	void *ptr = target;
	while (rem)
	{
		int av = 65536;
		if (av > rem)
			av = rem;
		
		int r = read(f, ptr, av);
		if (r < 0)
		{
			strcpy(progress, "read failed.");
			//goto a;
			return 0;
		}
		
		rem -= r;
		ptr += r;
		sprintf(progress, "Loading %s (%d%%)", filename, (s.st_size - rem) * 100 / s.st_size);
	}
	
	const char *r = filename;
	
	char *arg = memmem(target, s.st_size, r, strlen(r));
	if (arg)
	{
		strcpy(arg, argument);
	}

	sprintf(progress, "done reading...\n");
	
	stop = 1;

	mdelay(100);

	runelf();
}

