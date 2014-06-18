#include <rflpc17xx/rflpc17xx.h>

/*Define packet protocols headers*/
#include "protocols.h"

/*Define and init mac addr*/
uint8_t mac_addr[6]={0x00,0x00,0x00,0x00,0x00,0x00};

/*Define the rx descriptor and status structures*/
#define NUMBER_PACKET_RX 5
rflpc_eth_descriptor_t rx_descriptors[NUMBER_PACKET_RX];
rflpc_eth_rx_status_t  rx_status[NUMBER_PACKET_RX];


/*Define the tx descriptor and status structures*/
#define NUMBER_PACKET_TX 5
rflpc_eth_descriptor_t tx_descriptors[NUMBER_PACKET_TX];
rflpc_eth_tx_status_t  tx_status[NUMBER_PACKET_TX];

/*Define the rx and tx buffers*/
#define SIZE_BUFFER RFLPC_ETH_MAX_FRAME_LENGTH
uint8_t tx_buffers[NUMBER_PACKET_TX][SIZE_BUFFER];
uint8_t rx_buffers[NUMBER_PACKET_RX][SIZE_BUFFER];

void send_packet(){
  rflpc_eth_descriptor_t* p_descriptor = NULL;
  rflpc_eth_tx_status_t*  p_tx_status = NULL;

  /*Get the first free packet descriptor*/
  if( rflpc_eth_get_current_tx_packet_descriptor ( &p_descriptor, &p_tx_status, 0) ){
    

    /*Send the packet (online 1 descriptor)*/
    rflpc_eth_done_process_tx_packet (1); 
  }
}

void process_packet(rflpc_eth_descriptor_t* p_descriptor,  rflpc_eth_rx_status_t*  p_rx_status){
  ETHERNET_HEADER* h_ethernet = (ETHERNET_HEADER*)p_descriptor->packet;
  IP_HEADER* h_ip = (IP_HEADER*)(p_descriptor->packet+14);
  ICMP_HEADER* h_icmp = (ICMP_HEADER*)(p_descriptor->packet+34);

  /*If it is IPv4*/
  if( h_ethernet->ether_type[0] == 0x08 && h_ethernet->ether_type[1] == 0x00 ){

    /*ICMP protocol*/
    if(h_ip->protocol == 1){

      /*Ping*/
      if(h_icmp->type == 8 && h_icmp->code == 0)
	printf("PING from %i.%i.%i.%i to %i.%i.%i.%i\r\n",h_ip->ip_source[0],h_ip->ip_source[1],h_ip->ip_source[2],h_ip->ip_source[3],h_ip->ip_dest[0],h_ip->ip_dest[1],h_ip->ip_dest[2],h_ip->ip_dest[3]);

      /*Ping answer*/
      else if(h_icmp->type == 0 && h_icmp->code == 0)
	printf("REP from %i.%i.%i.%i to %i.%i.%i.%i\r\n",h_ip->ip_source[0],h_ip->ip_source[1],h_ip->ip_source[2],h_ip->ip_source[3],h_ip->ip_dest[0],h_ip->ip_dest[1],h_ip->ip_dest[2],h_ip->ip_dest[3]);

    }

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
    }
  }
  
}

void ethernet_init(){
  int i;

  /*Init ethernet device*/
  rflpc_eth_init();

  /*Set the device mac addr*/
  rflpc_eth_set_mac_address(mac_addr);  

  /*Wait for the link to be ready*/
  while(!rflpc_eth_link_state ());
 
  /*Initialize and sets rx descriptors and status base address*/
  for (i = 0 ; i < NUMBER_PACKET_RX ; i++){
    rx_descriptors[i].packet = rx_buffers[i];
    rx_descriptors[i].control = (SIZE_BUFFER - 1) | (1 << 31);
  }  
  rflpc_eth_set_rx_base_addresses ( rx_descriptors, rx_status, NUMBER_PACKET_RX); 

  /*Initialize and sets tx descriptors and status base address*/
  for (i = 0 ; i < NUMBER_PACKET_TX ; i++){
    tx_descriptors[i].packet = tx_buffers[i];
    tx_descriptors[i].control = (SIZE_BUFFER - 1) | (1 << 31);
  }  
  rflpc_eth_set_tx_base_addresses ( tx_descriptors, tx_status, NUMBER_PACKET_TX); 

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
  while(1);
 
  return 0;
}
