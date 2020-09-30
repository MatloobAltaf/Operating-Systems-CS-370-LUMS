#include<stdio.h>
#include<stdlib.h>

#define TOTAL_OUTER_TABLE_ENTRIES 64
#define TOTAL_INNER_TABLE_ENTRIES 256
#define MEMSIZE   131072
#define FRAME_SIZE 1024
#define TOTAL_FRAMES MEMSIZE/FRAME_SIZE
#define PAGE_SIZE  1024

int outer_page_number_mask = 0xff;
int inner_page_number_mask = 0xfff;
int offset_mask = 0xfc;

int* outer_page_table = NULL;
int** inner_pages = NULL;

int start_address = 0x00C17C00;
int end_address   = 0x00C193E8;

int filled_pages  = 0;
int free_memory_frame = 0;

FILE* store;

unsigned long * memory = NULL;

unsigned long getPhysicalAddress(unsigned int current_address, char** hit_miss)
{
	*hit_miss = "HIT";
	int offset   = current_address & 1023;
	int inner_pn = (current_address >> 10) & 0xff;
	int outer_pn = (current_address >> 18);

	if(outer_page_table[outer_pn] == -1)
	{
		//printf("---------------Outer page fault---------------\n");
		//outer pagetable fault -> load frame from backing store and update page tables

		fseek(store,inner_pn * PAGE_SIZE,SEEK_SET);
		fread(memory + (free_memory_frame * FRAME_SIZE),sizeof(long int),PAGE_SIZE,store);

		outer_page_table[outer_pn] = filled_pages;

		inner_pages[filled_pages] = malloc(sizeof(int) * TOTAL_INNER_TABLE_ENTRIES);


		for(int k = 0; k<TOTAL_INNER_TABLE_ENTRIES; k++)
		{
			inner_pages[filled_pages][k] = -1;
		}

		inner_pages[filled_pages][inner_pn] = free_memory_frame;

		filled_pages += 1;
		free_memory_frame += 1;
		*hit_miss = "MISS";

	}
	else
	{
		if(inner_pages[outer_page_table[outer_pn]][inner_pn] == -1)
		{
			//printf("-----------Inner page fault-----------\n");
			//inner pagetable fault -> load frame from backing store and update inner page table
			fseek(store,inner_pn * PAGE_SIZE,SEEK_SET);
			fread(memory + (free_memory_frame * FRAME_SIZE),sizeof(long int),PAGE_SIZE,store);

			inner_pages[outer_page_table[outer_pn]][inner_pn] = free_memory_frame;

			free_memory_frame += 1;
			*hit_miss = "MISS";
		}
		else if(filled_pages == TOTAL_OUTER_TABLE_ENTRIES)
		{
			//printf("-----------Pages full-----------\n");
			//swap out page and swap in required from backing store


			//swapping out

			//set dirty bits etc ----

			fseek(store,inner_pn * PAGE_SIZE,SEEK_SET);
			fwrite(memory + (inner_pages[outer_page_table[outer_pn]][inner_pn] * FRAME_SIZE),sizeof(long int),PAGE_SIZE,store);

			//swapping in
			// check dirty bits before loading in etc ----


			fseek(store,inner_pn * PAGE_SIZE,SEEK_SET);
			fread(memory + (free_memory_frame * FRAME_SIZE),sizeof(long int),PAGE_SIZE,store);
		}

	}

	return inner_pages[outer_page_table[outer_pn]][inner_pn] * FRAME_SIZE + offset;

}

void exec_code(unsigned long* code)
{
	printf("Instruction type	Address 1		Address 2	Value(before)  Value(after)\n");

	for(int i = 0; i<(end_address - start_address); i+=7)
	{
		char* instruction_type = "NULL";
		unsigned long instruction = code[i];
		int opcode = instruction >> (8 * 6);

		int address_1 = 0;
		int address_2 = 0;
		int value_aft = 0;
		int value_bef = 0;
		int value = 0;

		char* hit_miss = "NULL";

		if(opcode == 0x10 || opcode == 0x20 || opcode == 0x30 || opcode == 0x40 || opcode == 0x50 || opcode == 60 || opcode == 0x70)
		{
			instruction_type = "mem-val";
			address_2 = 0;
			address_1 = instruction >> ((8 * 4) & 0xfff);
			address_1 = getPhysicalAddress(address_1,&hit_miss);
			value = instruction & 0xff;
			value_bef = memory[address_1];
			value_aft = value_bef + value;
		}
		else if(opcode == 0x11 || opcode == 0x21 || opcode == 0x31 || opcode == 0x41 || opcode == 0x51 || opcode == 0x61 || opcode == 0x71)
		{
			instruction_type = "mem-mem";
			address_1 = instruction >> ((8 * 4) & 0xfff);
			address_2 = instruction >> ((8 * 1) & 0xfff);

			address_1 = getPhysicalAddress(address_1,&hit_miss);
			address_2 = getPhysicalAddress(address_2,&hit_miss);

			value_bef = memory[address_1];

			value_aft = value_bef + memory[address_2];
			memory[address_1] = value_aft;

		}

		if(instruction_type != "NULL")
		{
				printf("%s \t\t %x \t\t %s \t\t %x \t\t %d \t\t %d\n",instruction_type,address_1,hit_miss,address_2,value_bef,value_aft);
		}

		//printf("%x \t %s \t %x \t %x \t %d \t %d\n",opcode,instruction_type,address_1,address_2,value_bef,value_aft);

	}

}

int main(int argc, char** argv)
{

	if(argc < 2)
	{
		printf("Usage: ./run BACKING_STORE_2.bin\n");
		return 1;
	}

	store = fopen(argv[1],"ab+");



	memory = malloc(sizeof(unsigned long) * MEMSIZE);

	for(int i = 0; i<MEMSIZE; i++)
	{
		memory[i] = 0;
	}

	long int* code = malloc(sizeof(unsigned long) * (end_address - start_address));

	fseek(store,start_address,SEEK_SET);
	fread(code,sizeof(unsigned long),(end_address-start_address),store);

	outer_page_table = malloc(sizeof(int) * TOTAL_OUTER_TABLE_ENTRIES);
	inner_pages			= malloc(sizeof(int) * TOTAL_OUTER_TABLE_ENTRIES);

	for(int i = 0; i<TOTAL_OUTER_TABLE_ENTRIES; i++)
	{
		outer_page_table[i] = -1;
	}

	exec_code(code);
	fclose(store);

	return 0;
}
