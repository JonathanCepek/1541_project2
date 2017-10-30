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
#include <string.h> //added
#include "CPU.h" 

struct btb_entry {
	unsigned int entry_Addr;
	unsigned int btb_taken;
};

int main(int argc, char **argv)
{
  struct trace_item *fetch_entry=malloc(sizeof(struct trace_item)), *tr_entry=malloc(sizeof(struct trace_item));
  size_t size;
  char *trace_file_name;
  struct btb_entry btb_table[64];
  memset(btb_table, 0, (sizeof(struct btb_entry) * 64));
  
  
  //Buffer Declarations
  struct trace_item IF_ID;
  struct trace_item ID_EX;
  struct trace_item EX_MEM;
  struct trace_item MEM_WB;
  
  int trace_view_on = 0, added = 0, no_print = 0;
  int stop = -1,  end = 0, taken = 0, branch_cycle = 0; 
  unsigned int squashed[3];
  int branch_predictor = 0;

  unsigned int cycle_number = 0;

  if (argc == 2) {
    //Assume branch prediction field = 0 and trace = 0
    
    
  }
  else if (argc == 4) {
  	branch_predictor = atoi(argv[2]);
  	trace_view_on = atoi(argv[3]);
  }
  else {
 	fprintf(stdout, "\nUSAGE: tv <trace_file> <branch prediction (0/1)> <switch - (0/1)>\n");
    fprintf(stdout, "\n(branch) 0- predict not taken 1- use 1-bit branch detector.\n");
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

//Begin pipeline computation
  while(1) {
  
  	// Check for lw hazard DEBUG: should these be detected in the ID_EX buffer? DEBUG1
  	if( (EX_MEM.type == 3) && ((EX_MEM.dReg == ID_EX.sReg_a) || (EX_MEM.dReg == ID_EX.sReg_b)))
  	{
  		*fetch_entry = IF_ID;
  		
  		IF_ID = ID_EX;
  		
  		ID_EX.type = 0;		
   	}
   	else if( branch_predictor == 0 )
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
				for(int i = 0 ; i < 3 ; i++)
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
   	else if(branch_predictor == 1)
   	{
   		//Branch Detected, consult BTB
   		if(IF_ID.type == 5)
   		{
   			//printf("BRANCH DETECTED\n");
   			unsigned int b_pc;
			
			//Bitmask PC with 1111110000
			int index = (IF_ID.Addr & 1008) >> 4;
			//printf("Addr: %x\n", IF_ID.Addr);
			//printf("index: %d\n", index);
			//table entry matches
			if(btb_table[index].entry_Addr == IF_ID.Addr)
			{
				//printf("PC value found for %d\n", IF_ID.Addr);
				//printf("btb prediction: %d\n", btb_table[index].btb_taken);
				taken = btb_table[index].btb_taken;
			}
			else
			{
				//overwrite or set BTB entry
				//printf("Overwrite/Set\n");
				btb_table[index].entry_Addr = IF_ID.Addr;
				btb_table[index].btb_taken = 0; //predict not taken by default
			}
   		}
   	
   		if(EX_MEM.type == 5) //branch
		{
			//printf("BRANCH RESOLUTION:\n");
			//Check PC of Branch with instruction in ID buffer
			unsigned int b_pc, id_pc;
			b_pc = EX_MEM.PC;
			id_pc = ID_EX.PC;
			//printf("Addr: %x\n", EX_MEM.Addr);
			//printf("index: %d\n", (EX_MEM.Addr & 1008) >> 4);
			//Branch not taken
			if((id_pc - b_pc) == 4)
			{
				//prediction wrong, correct table
				if(taken == 1)
				{
					int fix_index = (EX_MEM.Addr & 1008) >> 4;
					btb_table[fix_index].btb_taken = 0;
					btb_table[fix_index].entry_Addr = EX_MEM.Addr; //DEBUG if needed or not
					//branch was taken, squash two loaded instructions DEBUG add this?
					for(int i = 0 ; i < 3 ; i++)
					{
						if(squashed[i] == 0)
						{
							squashed[i] = EX_MEM.PC;
							break;
						}
					}
				}
			}
			else //branch taken
			{
				//Prediction wrong
				if(taken == 0)
				{
					int fix_index = (EX_MEM.Addr & 1008) >> 4;
					btb_table[fix_index].btb_taken = 1;
					btb_table[fix_index].entry_Addr = EX_MEM.Addr; //DEBUG if needed or not
					//NOTE: no need to squash because trace is Dynamic
					//NOTEv2: The above assumption is wrong and why we lost points
					for(int i = 0 ; i < 3 ; i++)
					{
						if(squashed[i] == 0)
						{
							squashed[i] = EX_MEM.PC;
							break;
						}
					}					
				}
			}
		}
		
		size = trace_get_item(&fetch_entry);
   	}
   	else
   	{
    	size = trace_get_item(&fetch_entry);
   	}
   
    if (cycle_number == stop) {       /* no more instructions (trace_items) to simulate */
        printf("+ Simulation terminates at cycle : %u\n", cycle_number);
      	break;
    }
    else{           
		struct trace_item temp1, temp2;
		no_print = 0;
		//Copy first two buffers into temps
		temp1 = IF_ID;
		temp2 = ID_EX;
		//Bring new instruction into IF_ID buffer
		if (fetch_entry != 0 || size) //(size)
		{
			IF_ID = *fetch_entry;
		}
		
		//Propagate the old instructions to the next stage i.e. fetch_entry => IF_ID => ID_EX => EX_MEM => MEM_WB => tr_entry
		ID_EX = temp1;
		temp1 = EX_MEM;

		EX_MEM = temp2;
		
    	//*tr_entry = MEM_WB; //WTF
    	temp2 = MEM_WB;
    	
		MEM_WB = temp1;
		
		*tr_entry = temp2;
		
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
		      	
      	for(int j = 0 ; j < 3 ; j++)
      	{
      		if(squashed[j] == tr_entry->PC && cycle_number >= 5 && !end)
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
          break;
        case ti_STORE:
          printf("[cycle %d] STORE:",cycle_number) ;      
          printf(" (PC: %x)(sReg_a: %d)(sReg_b: %d)(addr: %x)\n", tr_entry->PC, tr_entry->sReg_a, tr_entry->sReg_b, tr_entry->Addr);
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
  }

  trace_uninit();

  exit(0);
}
