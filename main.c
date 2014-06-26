#include <rflpc17xx/rflpc17xx.h>

#include "protocols.h"
#include "arp_table.h"

ARP_TABLE ma_table;

/*Define the rx descriptor and status structures*/
#define NUMBER_PACKET_RX 5
rflpc_eth_descriptor_t rx_descriptors[NUMBER_PACKET_RX];
rflpc_eth_rx_status_t  rx_status[NUMBER_PACKET_RX];

/*Define the rx and tx buffers*/
#define SIZE_BUFFER RFLPC_ETH_MAX_FRAME_LENGTH
uint8_t rx_buffers[NUMBER_PACKET_RX][SIZE_BUFFER];

void process_packet(rflpc_eth_descriptor_t* p_descriptor,  rflpc_eth_rx_status_t*  p_rx_status){
  ETHERNET_HEADER* h_ethernet = (ETHERNET_HEADER*)p_descriptor->packet;
  ARP_HEADER* h_arp = (ARP_HEADER*)(p_descriptor->packet+14);
  IP_HEADER* h_ip = (IP_HEADER*)(p_descriptor->packet+14);
  COUPLE couple_source;
  COUPLE couple_dest;
  uint16_t res;

  /*Is it ARP ?*/
  if( h_ethernet->ether_type[0] == 0x08 && h_ethernet->ether_type[1] == 0x06 ){

    couple_source.Mac[0] = h_arp->sender_hardware_addr[0];
    couple_source.Mac[1] = h_arp->sender_hardware_addr[1];
    couple_source.Mac[2] = h_arp->sender_hardware_addr[2];
    couple_source.Mac[3] = h_arp->sender_hardware_addr[3];
    couple_source.Mac[4] = h_arp->sender_hardware_addr[4];
    couple_source.Mac[5] = h_arp->sender_hardware_addr[5];

    couple_dest.Mac[0] = h_arp->target_hardware_addr[0];
    couple_dest.Mac[1] = h_arp->target_hardware_addr[1];
    couple_dest.Mac[2] = h_arp->target_hardware_addr[2];
    couple_dest.Mac[3] = h_arp->target_hardware_addr[3];
    couple_dest.Mac[4] = h_arp->target_hardware_addr[4];
    couple_dest.Mac[5] = h_arp->target_hardware_addr[5];


    couple_source.IPv4[0] = h_arp->sender_logical_addr[0];
    couple_source.IPv4[1] = h_arp->sender_logical_addr[1];
    couple_source.IPv4[2] = h_arp->sender_logical_addr[2];
    couple_source.IPv4[3] = h_arp->sender_logical_addr[3];

    couple_dest.IPv4[0] = h_arp->target_logical_addr[0];
    couple_dest.IPv4[1] = h_arp->target_logical_addr[1];
    couple_dest.IPv4[2] = h_arp->target_logical_addr[2];
    couple_dest.IPv4[3] = h_arp->target_logical_addr[3];    
  }

  /*If it is IPv4*/
  else if( h_ethernet->ether_type[0] == 0x08 && h_ethernet->ether_type[1] == 0x00 ){
    couple_source.Mac[0] = h_ethernet->source[0];
    couple_source.Mac[1] = h_ethernet->source[1];
    couple_source.Mac[2] = h_ethernet->source[2];
    couple_source.Mac[3] = h_ethernet->source[3];
    couple_source.Mac[4] = h_ethernet->source[4];
    couple_source.Mac[5] = h_ethernet->source[5];

    couple_dest.Mac[0] = h_ethernet->dest[0];
    couple_dest.Mac[1] = h_ethernet->dest[1];
    couple_dest.Mac[2] = h_ethernet->dest[2];
    couple_dest.Mac[3] = h_ethernet->dest[3];
    couple_dest.Mac[4] = h_ethernet->dest[4];
    couple_dest.Mac[5] = h_ethernet->dest[5];


    couple_source.IPv4[0] = h_ip->ip_source[0];
    couple_source.IPv4[1] = h_ip->ip_source[1];
    couple_source.IPv4[2] = h_ip->ip_source[2];
    couple_source.IPv4[3] = h_ip->ip_source[3];

    couple_dest.IPv4[0] = h_ip->ip_dest[0];
    couple_dest.IPv4[1] = h_ip->ip_dest[1];
    couple_dest.IPv4[2] = h_ip->ip_dest[2];
    couple_dest.IPv4[3] = h_ip->ip_dest[3];
  }

  res = dicho_search_IPv4(ma_table,couple_dest);
  if(res != ma_table.nb_couple){
    if(comp_nbyte(ma_table.table[res].Mac,couple_dest.Mac,6)!=0)
      modify_mac(&ma_table, res, couple_dest.Mac);
  }
  else{
    res = dicho_search_Mac(ma_table,couple_dest);
    if(res != ma_table.nb_couple){
      if(comp_nbyte(ma_table.table[res].IPv4,couple_dest.IPv4,4)!=0)
	modify_IPv4(&ma_table, res, couple_dest.IPv4);
    }
    else
      add(&ma_table,couple_dest);
  }

  res = dicho_search_IPv4(ma_table,couple_source);
  if(res != ma_table.nb_couple){
    if(comp_nbyte(ma_table.table[res].Mac,couple_source.Mac,6)!=0)
      modify_mac(&ma_table, res, couple_source.Mac);
  }
  else{
    res = dicho_search_Mac(ma_table,couple_source);
    if(res != ma_table.nb_couple){
      if(comp_nbyte(ma_table.table[res].IPv4,couple_source.IPv4,4)!=0)
	modify_IPv4(&ma_table, res, couple_source.IPv4);
    }
    else
      add(&ma_table,couple_source);
  }
}


RFLPC_IRQ_HANDLER ethernet_callback(){
  rflpc_eth_descriptor_t* p_descriptor = NULL;
  rflpc_eth_rx_status_t*  p_rx_status = NULL;
  
  /*Is callback call by RFLPC_ETH_IRQ_EN_RX_DONE ?*/
  if( rflpc_eth_irq_get_status () & RFLPC_ETH_IRQ_EN_RX_DONE) {
  
    /*While there is packet to read*/
    while( rflpc_eth_rx_available() ) {

      /*Get the packet references*/
      rflpc_eth_get_current_rx_packet_descriptor(&p_descriptor , &p_rx_status);

      /*Processing*/
      process_packet(p_descriptor,p_rx_status);

      /*Drop the packet*/
      rflpc_eth_done_process_rx_packet();
      
      /*Stop the IRQ*/
      rflpc_eth_irq_clear (RFLPC_ETH_IRQ_EN_RX_DONE);
    }
  } 
}

void ethernet_init(){
  int i;

  printf("Init ethernet device\r\n");
  /*Init ethernet device*/
  rflpc_eth_init();
 
  printf("Wait for link to be up\r\n");
  /*Wait for the link to be ready*/
  while(!rflpc_eth_link_state ());
 
  printf("Set adresse for rx descriptor and status\r\n");
  /*Initialize and sets rx descriptors and status base address*/
  for (i = 0 ; i < NUMBER_PACKET_RX ; i++){
    rx_descriptors[i].packet = rx_buffers[i];
    rx_descriptors[i].control = (SIZE_BUFFER - 1) | (1 << 31);
  }  
  rflpc_eth_set_rx_base_addresses ( rx_descriptors, rx_status, NUMBER_PACKET_RX); 
  
  printf("Set irq callback\r\n");
  /*Enable IRQ when receiving*/
  rflpc_eth_irq_enable (RFLPC_ETH_IRQ_EN_RX_DONE);
  /*Set IRQ callback*/
  rflpc_eth_set_irq_handler (ethernet_callback);

  printf("Init ARP table\r\n");
  ma_table.nb_couple = 0;
  ma_table.index_time = 0;

  printf("Ethernet enabled\r\n");
}

RFLPC_IRQ_HANDLER uart_callback()
{
 if(rflpc_uart_getchar(RFLPC_UART0)=='p')
    show_arp_table(ma_table);
}

int main()
{
  /*Enable uart for printf*/
  rflpc_uart_init(RFLPC_UART0);

  /*Init and configure ethernet*/
  ethernet_init();

  /*Callback to get the instructions*/
  rflpc_uart_set_rx_callback(RFLPC_UART0, uart_callback);

  /*Infinite boucle*/
  while(1){
    //rflpc_idle;
  }
 
  return 0;
}
