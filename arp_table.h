#ifndef HEADER_ARP_TABLE
#define HEADER_ARP_TABLE

#include <rflpc17xx/rflpc17xx.h>

/*la valeur max de NB_MAX_COUPLE depend du type de nb_couple de la structure arp_table*/
#define NB_MAX_COUPLE 50

struct couple {
  uint8_t IPv4[4];
  uint8_t Mac[6];
  uint16_t time; 
};
typedef struct couple COUPLE;

/*Si le type(ie taille) de nb_couple est change le type de certaines varibles de dicho_search add et modify doit l'etre egalement*/
struct arp_table {
  COUPLE table[NB_MAX_COUPLE];
  uint16_t time[NB_MAX_COUPLE];
  uint16_t index_time;
  uint16_t nb_couple;
};
typedef struct arp_table ARP_TABLE;


/*A modifier, lors de l'ajout si plus de place enlever la dernière entree mise*/

uint16_t dicho_search_IPv4(ARP_TABLE table, COUPLE couple);
uint16_t dicho_search_Mac(ARP_TABLE table, COUPLE couple);
void add(ARP_TABLE* table, COUPLE couple);
void modify_IPv4(ARP_TABLE* table, uint16_t index, uint8_t IPv4[4]);
void modify_mac(ARP_TABLE* table, uint16_t index, uint8_t Mac[6]);
int8_t comp_nbyte(uint8_t* a, uint8_t* b, uint8_t n);
void show_arp_table(ARP_TABLE table);

#endif
