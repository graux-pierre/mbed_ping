#include <rflpc17xx/rflpc17xx.h>
#include "arp_table.h"

#define COMP(a,b) if((a) > (b)) return 1; \
  else if((a) < (b)) return -1;

int8_t comp_nbyte(uint8_t* a, uint8_t* b, uint8_t n){
  uint8_t i;

  for(i=0;i<n;i++){
    COMP(a[i],b[i]);
  }
  return 0;
}

uint16_t dicho_search_IPv4(ARP_TABLE table, COUPLE couple){
  uint16_t begin, mid ,end;
  int8_t comp;

  if(table.nb_couple != 0){
    
    begin = 0;
    end = table.nb_couple-1;
    while(end >= begin){
      mid = (end-begin)/2 + begin;
      
      comp = comp_nbyte(table.table[mid].IPv4, couple.IPv4, 4);
      if(comp == 0)
	return mid; 
      else if(comp == -1){
	if(mid==table.nb_couple)
	  return table.nb_couple;
	begin = mid+1;
      }
      else if(comp == 1){
	if(mid==0)
	  return table.nb_couple;
	end = mid-1;
      }
    }
  }
  
  return table.nb_couple;
}


uint16_t dicho_search_Mac(ARP_TABLE table, COUPLE couple){
  uint16_t i;
 
  for(i=0;i<table.nb_couple;i++){
    if(comp_nbyte(table.table[i].Mac,couple.Mac,6)==0)
      return i;
  }

  return table.nb_couple;
}

void add(ARP_TABLE* table, COUPLE couple){
  int16_t i;
  COUPLE tmp;

  /*add if table is full*/
  if(table->nb_couple == NB_MAX_COUPLE){
        
    table->index_time++;
    if(table->index_time == NB_MAX_COUPLE)
      table->index_time = 0;

    couple.time = table->index_time;


    i = table->time[table->index_time];
    
    if(comp_nbyte(table->table[i].IPv4,couple.IPv4,4) != 1){
      
      for(i++; i<NB_MAX_COUPLE&&comp_nbyte(table->table[i].IPv4,couple.IPv4,4) != 1;i++){
	table->table[i-1] = table->table[i];
	table->time[table->table[i-1].time]=i-1;	 
      }
      table->table[i-1] = couple;
      table->time[couple.time] = i-1;
    }
    else {
      for(i--; i>=0 && comp_nbyte(table->table[i].IPv4,couple.IPv4,4) == 1;i--){
	table->table[i+1] = table->table[i];
	table->time[table->table[i+1].time]=i+1;	 
      }
      table->table[i+1] = couple;
      table->time[couple.time] = i+1;
    }
  }
  /*add if table is not full*/
  else {
    for(i=0;i<table->nb_couple && comp_nbyte(table->table[i].IPv4,couple.IPv4,4) != 1;i++);
    
    if(comp_nbyte(table->table[i].IPv4,couple.IPv4,4) != 1){
      couple.time = table->index_time;
      table->time[table->index_time] = table->nb_couple;
      table->table[table->nb_couple] = couple;
    }
    else{

      couple.time = table->index_time;
      table->time[table->index_time]= i;
      for(;i<table->nb_couple;i++){
	tmp = table->table[i];
	table->table[i] = couple;
	couple = tmp;
	table->time[couple.time] = i+1;

      }


      table->table[table->nb_couple] = couple;    
    }
    
    table->nb_couple++;
    table->index_time++;
    if(table->index_time==NB_MAX_COUPLE)
      table->index_time = 0;
  }
}

void modify_IPv4(ARP_TABLE* table, uint16_t index, uint8_t IPv4[4]){
  table->table[index].IPv4[0] = IPv4[0];
  table->table[index].IPv4[1] = IPv4[1];
  table->table[index].IPv4[2] = IPv4[2];
  table->table[index].IPv4[3] = IPv4[3];
}

void modify_mac(ARP_TABLE* table, uint16_t index, uint8_t Mac[6]){
  table->table[index].Mac[0] = Mac[0];
  table->table[index].Mac[1] = Mac[1];
  table->table[index].Mac[2] = Mac[2];
  table->table[index].Mac[3] = Mac[3];
  table->table[index].Mac[4] = Mac[4];
  table->table[index].Mac[5] = Mac[5];
}

void show_arp_table(ARP_TABLE table){
  uint16_t i;

  printf("ARP Table :\r\n");

  for(i=0;i<NB_MAX_COUPLE;i++)
    printf("%i.%i.%i.%i <-> %X:%X:%X:%X:%X:%X\r\n",table.table[i].IPv4[0],table.table[i].IPv4[1],table.table[i].IPv4[2],table.table[i].IPv4[3],table.table[i].Mac[0],table.table[i].Mac[1],table.table[i].Mac[2],table.table[i].Mac[3],table.table[i].Mac[4],table.table[i].Mac[5]);

  printf("\r\n");
}
