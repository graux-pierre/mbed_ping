#include <rflpc17xx/rflpc17xx.h>

/*Define packet protocols headers*/
#include "protocols.h"

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
  ICMP_HEADER* h_icmp = (ICMP_HEADER*)(p_descriptor->packet+34); 
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

  printf("Ethernet enabled\r\n");
}

int main()
{
  /*Enable uart for printf*/
  rflpc_uart_init(RFLPC_UART0);

  /*Init and configure ethernet*/
  ethernet_init();
  
  /*Infinite boucle*/
  while(1)
      rflpc_idle;
 
  return 0;
}
