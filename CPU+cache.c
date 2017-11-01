/**************************************************************/
/* CS/COE 1541				 			
   just compile with gcc -o pipeline pipeline.c			
   and execute using							
   ./pipeline  /afs/cs.pitt.edu/courses/1541/short_traces/sample.tr	0  
***************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <string.h>
#include "CPU.h"

// to keep cache statistics
unsigned int I_accesses = 0;
unsigned int I_misses = 0;
unsigned int D_read_accesses = 0;
unsigned int D_read_misses = 0;
unsigned int D_write_accesses = 0; 
unsigned int D_write_misses = 0; 

#include "cache.h" 

int main(int argc, char **argv)
{
  struct trace_item *fetch_entry=malloc(sizeof(struct trace_item)), *tr_entry=malloc(sizeof(struct trace_item));
  size_t size;
  char *trace_file_name;
  unsigned int I_size, I_bsize, I_assoc, D_size, D_bsize, D_assoc, mem_latency;
  
  
  //Buffer Declarations
  struct trace_item IF_ID;
  struct trace_item ID_EX;
  struct trace_item EX_MEM;
  struct trace_item MEM_WB;
  
  int trace_view_on = 0, added = 0, no_print = 0;
  int stop = -1,  end = 0, branch_cycle = 0; 
  unsigned int squashed[3];
  int branch_predictor = 0;

  unsigned int cycle_number = 0;

  if (argc == 2) {
	//Assume trace = 0
	
  }
  else if (argc == 3) trace_view_on = atoi(argv[2]);
  else {
	fprintf(stdout, "\nUSAGE: tv <trace_file> <switch - (0/1)>\n");
	fprintf(stdout, "\n(switch) to turn on or off individual item view.\n\n");
	exit(0);
  }
	
  trace_file_name = argv[1];

  fprintf(stdout, "\n ** opening file %s\n", trace_file_name);

  trace_fd = fopen(trace_file_name, "rb");

  if (!trace_fd) {
	fprintf(stdout, "\ntrace file %s not opened.\n\n", trace_file_name);
	exit(0);
  }
  
  trace_init();
  
  //read config file
  fprintf(stdout, "\n ** opening file cache_config.txt\n");

  config_fd = fopen("cache_config.txt", "r");
  
  if (!config_fd) {
	 fprintf(stdout, "\nconfig file not opened.\n\n");
	exit(0);
  }

  //read config file
  fscanf(config_fd, "%u\n%u\n%u\n%u\n%u\n%u\n%u\n", &I_size, &I_assoc, &I_bsize, &D_size, &D_assoc, &D_bsize, &mem_latency);
  
  fclose(config_fd);
  
  //Create caches
  struct cache_t *I_cache, *D_cache;
  I_cache = cache_create(I_size, I_bsize, I_assoc, mem_latency); 
  D_cache = cache_create(D_size, D_bsize, D_assoc, mem_latency);

//Begin pipeline computation
  while(1) {
  
	// Check for lw hazard DEBUG: should these be detected in the ID_EX buffer? DEBUG1
	if( (EX_MEM.type == 3) && ((EX_MEM.dReg == ID_EX.sReg_a) || (EX_MEM.dReg == ID_EX.sReg_b)))
	{
		*fetch_entry = IF_ID;
		
		IF_ID = ID_EX;
		
		ID_EX.type = 0;		
	}
	else
	{
		if( EX_MEM.type == 5) //branch
		{
			//Check PC of Branch with instruction in ID buffer
			unsigned int b_pc, id_pc;
			b_pc = EX_MEM.PC;
			id_pc = ID_EX.PC;
			
			//printf("Check Branch Condition\n");
			
			if((id_pc - b_pc) != 4)
			{
				//branch taken, squash first two buffers
				int i;
				for(i = 0 ; i < 3 ; i++)
				{
					if(squashed[i] == 0)
					{
						squashed[i] = EX_MEM.PC;
						break;
					}
				}
			}
		}
		
		size = trace_get_item(&fetch_entry);
	}
   
	if (cycle_number >= stop) {       /* no more instructions (trace_items) to simulate */
		printf("+ Simulation terminates at cycle : %u\n", cycle_number);
		printf("I-cache accesses %u and misses %u\n", I_accesses, I_misses);
		printf("D-cache Read accesses %u and misses %u\n", D_read_accesses, D_read_misses);
		printf("D-cache Write accesses %u and misses %u\n", D_write_accesses, D_write_misses);
		break;
	}
	else{           
		no_print = 0;
		
		//Propagate in reverse for simplicity
		*tr_entry = MEM_WB;
		MEM_WB = EX_MEM;
		EX_MEM = ID_EX;
		ID_EX = IF_ID;
		if (fetch_entry != 0 || size) //(size)
		{
			IF_ID = *fetch_entry;
		}
		
		//Print Current Cycle instructions
		//printf("---------Cycle:  %d---------\n", cycle_number);
		//printf("IF_ID|| type: %d\n", IF_ID.type);
		//printf("ID_EX|| type: %d\n", ID_EX.type);
		//printf("EX_MEM|| type: %d\n", EX_MEM.type);
		//printf("MEM_WB|| type: %d\n", MEM_WB.type);
		//printf("---------------------------\n\n");
				
		//clear the pipeline after the size returns an empty string
		if(!size && end == 0)
		{
			end = 1;
			stop = cycle_number + 4;
		}
			
		cycle_number++;	
	int j;
		for(j = 0 ; j < 3 ; j++)
		{
			if(squashed[j] == tr_entry->PC && cycle_number >= 5 && !end) //DEBUG ugly condition, consider commenting
			{
				if(trace_view_on)
				{
					printf("[cycle %d] BRANCH:",cycle_number) ;
		      		printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
					printf("[cycle %d] SQUASHED\n",++cycle_number);
					printf("[cycle %d] SQUASHED\n",++cycle_number);
				}
				else
				{
					cycle_number = cycle_number + 2;
				}
				squashed[j] = 0;
				no_print = 1;
			}
		}	
	}  
	int poop = 0;
	int temp = cycle_number;
	cycle_number = cycle_number + cache_access(I_cache, tr_entry->PC, 0); /* simulate instruction fetch */
	// update I_access and I_misses
	I_accesses++;
	if(cycle_number > temp) I_misses++;
	if (trace_view_on && !no_print && cycle_number >= 5) {/* print the executed instruction if trace_view_on=1 and don't print the first cycle's initial value*/
	  switch(tr_entry->type) {
		case ti_NOP:
		  printf("[cycle %d] NOP:\n",cycle_number) ;
		  break;
		case ti_RTYPE:
		  printf("[cycle %d] RTYPE:",cycle_number) ;
		  printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(dReg: %d) \n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->dReg);
		  break;
		case ti_ITYPE:
		  printf("[cycle %d] ITYPE:",cycle_number) ;
		  printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
		  break;
		case ti_LOAD:
		  printf("[cycle %d] LOAD:",cycle_number) ;      
		  printf(" (PC: %x)(sReg_a: %d)(dReg: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->dReg, tr_entry->Addr);
		  temp = cycle_number;
		  cycle_number = cycle_number + cache_access(D_cache, tr_entry->Addr, 0);
		  // update D_read_access and D_read_misses
		  D_read_accesses++;
		  if(cycle_number > temp) D_read_misses++;
		  break;
		case ti_STORE:
		  printf("[cycle %d] STORE:",cycle_number) ;      
		  printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
		  temp = cycle_number;
		  cycle_number = cycle_number + cache_access(D_cache, tr_entry->Addr, 1);
		  // update D_write_access and D_write_misses 
		  D_write_accesses++;  
		  if(cycle_number > temp) D_write_misses++;       
		  break;
		case ti_BRANCH:
		  printf("[cycle %d] BRANCH:",cycle_number) ;
		  printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
		  break;
		case ti_JTYPE:
		  printf("[cycle %d] JTYPE:",cycle_number) ;
		  printf(" (PC: %x)(addr: %x)\n", tr_entry->PC,tr_entry->Addr);
		  break;
		case ti_SPECIAL:
		  printf("[cycle %d] SPECIAL:\n",cycle_number) ;      	
		  break;
		case ti_JRTYPE:
		  printf("[cycle %d] JRTYPE:",cycle_number) ;
		  printf(" (PC: %x) (sReg_a: %d)(addr: %x)\n", tr_entry->PC, tr_entry->dReg, tr_entry->Addr);
		  break;
	  }

	}
	else if(cycle_number >= 5)
	{
		if(tr_entry->type == ti_LOAD)
		{
			temp = cycle_number;
			poop = cache_access(D_cache, tr_entry->Addr, 0);
			cycle_number = cycle_number + poop;
			// update D_read_access and D_read_misses
			D_read_accesses++;
			if(cycle_number > temp) D_read_misses++;
		}
		else if (tr_entry->type == ti_STORE)
		{
		  temp = cycle_number;
		  poop = cache_access(D_cache, tr_entry->Addr, 1);
		  cycle_number = cycle_number + poop;
		  // update D_write_access and D_write_misses 
		  D_write_accesses++;  
		  if(cycle_number > temp) D_write_misses++;   
		}
	}
  }

  trace_uninit();

  exit(0);
}

