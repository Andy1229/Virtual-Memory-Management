#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#define MAX_QUEUE_LEN 1000000
/*Event struct*/
typedef struct Event {
	int page;
	char mod;
}Event;
/*memory struct*/
typedef struct Memory{
	int page;
	char mod;
	int ref;
	int age; /*use for FIFO*/
}Memory;

/*convert a hex string to decimal int*/
int Hex2Decimal(char *hex);
/*init memory queue*/
Memory *initMemory(int NUMPAGES);
/*read the event file to Event queue*/
void readEventsFile(char *filename, Event *e,int *events_size,int PAGESIZE);
/*SC */
void SC(Event *events, int events_size,int NUMPAGES);
/*ESC */
void ESC(Event *events, int events_size, int NUMPAGE);
/*ARB */
void ARB(Event *events, int events_size, int NUMPAGE,int INTERVAL);
/*EARB */
void EARB(Event *events, int events_size, int NUMPAGE, int INTERVAL);
/*print result*/
void printResult(int events, int read, int write);
int main(int argc,char** argv)
{
	Event *events = (Event*)malloc(MAX_QUEUE_LEN*sizeof(Event));
	int events_size; 
	readEventsFile(argv[1],events,&events_size, atoi(argv[2]));
	if (strcmp(argv[4],"SC")==0)
		SC(events, events_size, atoi(argv[3]));
	else if (strcmp(argv[4], "ESC") == 0)
		ESC(events, events_size, atoi(argv[3]));
	else if (strcmp(argv[4], "ARB") == 0)
		ARB(events, events_size, atoi(argv[3]), atoi(argv[5]));
	else if (strcmp(argv[4], "EARB") == 0)
		EARB(events, events_size, atoi(argv[3]), atoi(argv[5]));
	else return -1;
//    getchar();
    return 0;
}

void readEventsFile(char *filename, Event *e,int *e_size,int PAGESIZE)
{
	FILE *fin = fopen(filename,"r");
	char s[100];
	*e_size = 0;
	while (fgets(s,100,fin))
	{
		if (s[0] == 'R' || s[0] == 'W')
		{
			char tmp[9];
			sscanf(s, "%c%s", &e[*e_size].mod, tmp);
			e[*e_size].page = Hex2Decimal(tmp)/PAGESIZE;
			++(*e_size);
		}
	}
}

int Hex2Decimal(char *hex)
{
	int len = strlen(hex);
	unsigned num = 0;
	int shift;

	for (int i = 0, temp = 0; i < len; i++, temp = 0)
	{
		char c = *(hex + i);
		if (isalpha(c))
			temp = isupper(c) ? c - 55 : c - 87;
		if (isdigit(c))
			temp = c - '0';
		shift = (len - i - 1) * 4;
		temp = temp << shift;
		num = num | temp;
	}
	return num;
}

Memory * initMemory(int NUMPAGES)
{
	Memory *m = malloc(NUMPAGES * sizeof(Memory));
	for (int i = 0; i < NUMPAGES; ++i)
	{
		m[i].page = -1;
		m[i].ref = -1; /*-1 is no page in this memory*/
		m[i].mod = 0;
	}
	return m;
}

void SC(Event *events, int events_size, int NUMPAGES)
{
	int disk_reads_count = 0, disk_writes_count = 0; /*result count*/
	int m_ptr = 0; /*current memory pointer*/
	Memory *memory = initMemory(NUMPAGES);
	for (int i = 0; i < events_size; ++i) /*for each page in event queue*/
	{
		int hit = 0; 
		for (int j = 0; j < NUMPAGES; ++j)
		{
			if (memory[j].ref != -1 && memory[j].page == events[i].page)
			{//hit this page
				/*if hit a no dirty memory ,change memory state*/
				memory[j].mod = memory[j].mod == 'R'?events[i].mod:memory[j].mod;
				memory[j].ref = 1;
				hit = 1;
				break;
			}
		}
		if (!hit)
		{
			/*if not hit ,read_count +1*/
			++disk_reads_count;
			if (memory[m_ptr].ref == -1)
			{/*if the memory is empty,add page to this memory*/
				memory[m_ptr].page = events[i].page;
				memory[m_ptr].ref = 1;
				memory[m_ptr].mod = events[i].mod;
				m_ptr = (m_ptr + 1) % NUMPAGES;
			}
			else 
			{/*no hit and memory is not empty,need replace*/
				while (1)
				{/*use second chance*/
					if (memory[m_ptr].ref == 1)
					{/*this memory can't be repalce*/
						memory[m_ptr].ref = 0;
						m_ptr = (m_ptr + 1) % NUMPAGES;
					}
					else
					{
						if (memory[m_ptr].mod == 'W') /*if this memory is dirty,write_count +1*/
							++disk_writes_count;
						memory[m_ptr].page = events[i].page;
						memory[m_ptr].ref = 1;
						memory[m_ptr].mod = events[i].mod;
						m_ptr = (m_ptr + 1) % NUMPAGES;
						break;
					}
				}
			}
		}
	}
	printResult(events_size, disk_reads_count, disk_writes_count);
}

void ESC(Event *events, int events_size, int NUMPAGES)
{
	int disk_reads_count = 0, disk_writes_count = 0; /*result count*/
	int m_ptr = 0; /*current memory pointer*/
	Memory *memory = initMemory(NUMPAGES);
	for (int i = 0; i < events_size; ++i)
	{
		int hit = 0; /*hit flag*/
		for (int j = 0; j < NUMPAGES; ++j)
		{
			if (memory[j].ref != -1 && memory[j].page == events[i].page)
			{//hit this page
				/*if hit a no dirty memory ,change memory state*/
				memory[j].mod = memory[j].mod == 'R' ? events[i].mod : memory[j].mod;
				memory[j].ref = 1;
				hit = 1;
				break;
			}
		}
		if (!hit) {
			/*if not hit ,read_count +1*/
			++disk_reads_count;
			if (memory[m_ptr].ref == -1)
			{/*if the memory is empty,add page to this memory*/
				memory[m_ptr].page = events[i].page;
				memory[m_ptr].ref = 1;
				memory[m_ptr].mod = events[i].mod;
				m_ptr = (m_ptr + 1) % NUMPAGES;
			}
			else
			{
				int find = 0;
				int begin = m_ptr;
				while (!find)
				{/*use enhance second chance ,if find a replace pos,end this loop*/
					do
					{//loop 1: looking for reference bit =0,mode=read
						if (memory[m_ptr].ref == 0 && memory[m_ptr].mod == 'R')
						{
							memory[m_ptr].page = events[i].page;
							memory[m_ptr].mod = events[i].mod;
							memory[m_ptr].ref = 1;
							m_ptr = (m_ptr + 1) % NUMPAGES;
							find = 1;
							break;
						}
						else
							m_ptr = (m_ptr + 1) % NUMPAGES;
					} while (m_ptr != begin);
					while (!find)
					{//loop2:looking for reference bit =0,mode=write
						if (memory[m_ptr].ref == 1)
						{
							memory[m_ptr].ref = 0;
							m_ptr = (m_ptr + 1) % NUMPAGES;
						}
						else {
							++disk_writes_count;
							memory[m_ptr].page = events[i].page;
							memory[m_ptr].ref = 1;
							memory[m_ptr].mod = events[i].mod;
							m_ptr = (m_ptr + 1) % NUMPAGES;
							find = 1;
							break;
						}
						if (m_ptr == begin)/*if all memory if be scaned ,end the loop*/
							break;
					}
				}
			}
		}
	}
	printResult(events_size, disk_reads_count, disk_writes_count);
}

void ARB(Event *events, int events_size, int NUMPAGES, int INTERVAL)
{
	int disk_reads_count = 0, disk_writes_count = 0; /*result count*/
	int m_ptr = 0; /*current memory pointer*/
	Memory *memory = initMemory(NUMPAGES);
	for (int i = 0; i < events_size; ++i)
	{
		if (i  % INTERVAL == 0)/*shift reference*/
			for (int j = 0; j < NUMPAGES; ++j)
				if (memory[j].ref != -1)
					memory[j].ref >>= 1;
		int hit = 0;
		for (int j = 0; j < NUMPAGES; ++j)
			if (memory[j].ref != -1 && memory[j].page == events[i].page)
			{/*hit this page*/
				/*if hit a no dirty memory ,change memory state*/
				memory[j].mod = memory[j].mod == 'R' ? events[i].mod : memory[j].mod;
				memory[j].ref |= 128; /*new ref bit is 10000000 | old ref bit*/
				hit = 1;
				break;
			}
		if (!hit) {
			/*not hit a memory,read_count +1*/
			++disk_reads_count;
			if (memory[m_ptr].ref == -1)
			{/*this memory no data*/
				memory[m_ptr].page = events[i].page;
				memory[m_ptr].ref = 128;
				memory[m_ptr].mod = events[i].mod;
				memory[m_ptr].age = 0;
				m_ptr = (m_ptr + 1) % NUMPAGES;
			}
			else
			{/*all memory have data and not be hit*/
				Memory old_memory = memory[0]; /*record the oldest memory*/
				int o_ptr = 0;/*record the oldest memory pionter*/
				/*search oldest memory ,if reference bits are same,choose one use FIFO*/
				for (int j = 1; j != NUMPAGES; ++j)
				{
					if (old_memory.ref > memory[j].ref)
						old_memory = memory[j], o_ptr = j;
					if (old_memory.ref == memory[j].ref && old_memory.age < memory[j].age) /*use age to implement FIFO*/
						old_memory = memory[j], o_ptr = j;
				}
				if (memory[o_ptr].mod == 'W')/*if memory mode is write,write_count +1*/
					++disk_writes_count;
				/*replace memory to current page*/
				memory[o_ptr].page = events[i].page;
				memory[o_ptr].mod = events[i].mod;
				memory[o_ptr].ref = 128;
				memory[o_ptr].age = 0;
			}
		}
		for (int j = 0; j < NUMPAGES; ++j)
			++memory[j].age;
	}
	printResult(events_size, disk_reads_count, disk_writes_count);
}

void EARB(Event *events, int events_size, int NUMPAGES, int INTERVAL)
{
	int disk_reads_count = 0, disk_writes_count = 0; /*result count*/
	int m_ptr = 0; /*current memory pointer*/
	Memory *memory = initMemory(NUMPAGES);
	for (int i = 0; i < events_size; ++i)/*shift reference*/
	{
		if (i%INTERVAL == 0)
			for (int j = 0; j < NUMPAGES; ++j)
				if (memory[j].ref != -1)
					memory[j].ref >>= 1;
		int hit = 0;
		for (int j = 0; j < NUMPAGES; ++j)
		{
			if (memory[j].page == events[i].page)
			{
				/*if hit a no dirty memory ,change memory state*/
				memory[j].mod = memory[j].mod == 'R' ? events[i].mod : memory[j].mod;
				memory[j].ref |= 128;
				hit = 1;
				break;
			}
		}
		if (hit == 0) {
			/*no hit,read_count+1*/
			++disk_reads_count;
			if (memory[m_ptr].ref == -1)
			{/*memory is empty*/
				memory[m_ptr].page = events[i].page;
				memory[m_ptr].mod = events[i].mod;
				memory[m_ptr].ref = 128;
				memory[m_ptr].age = 0;
				m_ptr = (m_ptr + 1) % NUMPAGES;
			}
			else
			{/*all memory have data and not be hit*/
				Memory old_memory = memory[0]; /*record the oldest memory*/
				int o_ptr = 0;/*record the oldest memory pos*/
				/*search oldest use EARB*/
				for (int j = 1; j != NUMPAGES; ++j)
				{
					if (old_memory.mod == memory[j].mod)
					{/*If no pages are modified, or if only modified pages are resident, the algorithm should perform the
					 same as ARB.*/
						if (old_memory.ref > memory[j].ref)
							old_memory = memory[j], o_ptr = j;
						if (old_memory.ref == memory[j].ref && old_memory.age < memory[j].age) /*use age to implement FIFO*/
							old_memory = memory[j], o_ptr = j;
					}
					/*If both modified and unmodified pages are resident:*/
					else if (old_memory.mod == 'W')
					{
						if (memory[j].ref <= (old_memory.ref ? old_memory.ref << 4 : 8))
							old_memory = memory[j], o_ptr = j;
					}
					else
					{
						if ((memory[j].ref ? memory[j].ref << 4 : 8) < old_memory.ref)
							old_memory = memory[j], o_ptr = j;
					}
				}
				if (memory[o_ptr].mod == 'W')/*if memory mode is write,write_count+1*/
					++disk_writes_count;
				/*replace memory to current page*/
				memory[o_ptr].page = events[i].page;
				memory[o_ptr].ref = 128;
				memory[o_ptr].mod = events[i].mod;
				memory[o_ptr].age = 0;
			}
		}
		for (int j = 0; j < NUMPAGES; ++j)
			++memory[j].age;
	}
	printResult(events_size, disk_reads_count, disk_writes_count);
}

void printResult(int events, int read, int write)
{
	printf("events in trace:\t%d\n", events);
	printf("total disk reads:\t%d\n", read);
	printf("total disk writes:\t%d\n", write);
}
