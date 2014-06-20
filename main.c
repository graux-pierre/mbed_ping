#include <rflpc17xx/rflpc17xx.h>

/*Define packet protocols headers*/
#include "protocols.h"

/*Define and init mac addr and ip addr*/
uint8_t mac_addr[6]={0x00,0x1e,0xc9,0x04,0x05,0x06};
uint8_t ip_addr[4]={134,206,100,200};

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

void reply_arp(uint8_t mac_dest[6], uint8_t ip_dest[4]){
  rflpc_eth_descriptor_t* p_descriptor = NULL;
  rflpc_eth_tx_status_t*  p_tx_status = NULL;

  /*Get the first free packet descriptor*/
  if( rflpc_eth_get_current_tx_packet_descriptor ( &p_descriptor, &p_tx_status, 0) ){
   
    /*ARP request size : 42, IRQ enable, last fragment, CRC*/
    p_descriptor->control = 41 | (1<<30) | (1<<29) ;
    
    /*No status information*/
    p_tx_status->status_info = 0;
    
    /*Create an arp request*/
    arp_request((uint8_t*)p_descriptor->packet,mac_dest,ip_dest);

    /*Send the packet (only 1 descriptor)*/
    rflpc_eth_done_process_tx_packet (1); 
    printf("Reply arp to %i.%i.%i.%i\n\r",ip_dest[0],ip_dest[1],ip_dest[2],ip_dest[3]);
  }
}

void send_ping(){
  rflpc_eth_descriptor_t* p_descriptor = NULL;
  rflpc_eth_tx_status_t*  p_tx_status = NULL;
  uint8_t mac_dest[6] = {0x00,0x1e,0xc9,0x39,0x5f,0x96};
  uint8_t ip_dest[4] = {134,206,90,102};

  /*Get the first free packet descriptor*/
  if( rflpc_eth_get_current_tx_packet_descriptor ( &p_descriptor, &p_tx_status, 0) ){

    /*ICMP request size : 42, IRQ enable, last fragment, CRC*/
    p_descriptor->control = 41 | (1<<30) | (1<<29) ;
    
    /*No status information*/
    p_tx_status->status_info = 0;
    
    /*Create a ping request*/
    ping_request((uint8_t*)p_descriptor->packet,mac_dest,ip_dest);

    /*Send the packet (only 1 descriptor)*/
    rflpc_eth_done_process_tx_packet (1); 
    printf("Ping sent\n\r");
  }
}

void send_pong(uint8_t mac_dest[6], uint8_t ip_dest[4], uint8_t id[2], uint8_t seq[2], const uint8_t* data, uint8_t data_size){
  rflpc_eth_descriptor_t* p_descriptor = NULL;
  rflpc_eth_tx_status_t*  p_tx_status = NULL;
  
  /*Get the first free packet descriptor*/
  if( rflpc_eth_get_current_tx_packet_descriptor ( &p_descriptor, &p_tx_status, 0) ){

    /*ICMP request size : 42, IRQ enable, last fragment, CRC*/
    p_descriptor->control = (41+data_size) | (1<<30) | (1<<29) ;
    
    /*No status information*/
    p_tx_status->status_info = 0;

    /*Create a ping request*/
    pong_request((uint8_t*)p_descriptor->packet,mac_dest,ip_dest,id,seq,data,data_size);

    /*Send the packet (only 1 descriptor)*/
    rflpc_eth_done_process_tx_packet (1); 
    printf("Pong sent\n\r");
  }
}

void process_packet(rflpc_eth_descriptor_t* p_descriptor,  rflpc_eth_rx_status_t*  p_rx_status){
  ETHERNET_HEADER* h_ethernet = (ETHERNET_HEADER*)p_descriptor->packet;
  ARP_HEADER* h_arp = (ARP_HEADER*)(p_descriptor->packet+14);
  IP_HEADER* h_ip = (IP_HEADER*)(p_descriptor->packet+14);
  ICMP_HEADER* h_icmp = (ICMP_HEADER*)(p_descriptor->packet+34);
  
  /*Is it ARP ?*/
  if( h_ethernet->ether_type[0] == 0x08 && h_ethernet->ether_type[1] == 0x06 ){
    /*Is it a request?*/
    if( h_arp->operation[1] == 1 ){
      /*Asking me ?*/
      if(h_arp->target_logical_addr[0] == ip_addr[0] && h_arp->target_logical_addr[1] == ip_addr[1] && h_arp->target_logical_addr[2] == ip_addr[2] && h_arp->target_logical_addr[3] == ip_addr[3])
	reply_arp(h_arp->sender_hardware_addr,h_arp->sender_logical_addr);
    }
  }
  
  /*If it is IPv4*/
  if( h_ethernet->ether_type[0] == 0x08 && h_ethernet->ether_type[1] == 0x00 ){

    /*ICMP protocol*/
    if(h_ip->protocol == 1){
      
      /*Ping*/
      if(h_icmp->type == 8 && h_icmp->code == 0){
	if(h_ip->ip_dest[0] == ip_addr[0] && h_ip->ip_dest[1] == ip_addr[1] && h_ip->ip_dest[2] == ip_addr[2] && h_ip->ip_dest[3] == ip_addr[3]){
	  printf("PING\r\n");
	  send_pong(h_ethernet->source,h_ip->ip_source,h_icmp->id,h_icmp->seq_num,(const uint8_t*)(p_descriptor->packet+42),(h_ip->total_length[0]<<8)+h_ip->total_length[1]-28);
	}
      }

      /*Ping answer*/
      else if(h_icmp->type == 0 && h_icmp->code == 0)
	printf("PONG from %i.%i.%i.%i to %i.%i.%i.%i\r\n",h_ip->ip_source[0],h_ip->ip_source[1],h_ip->ip_source[2],h_ip->ip_source[3],h_ip->ip_dest[0],h_ip->ip_dest[1],h_ip->ip_dest[2],h_ip->ip_dest[3]);

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
 
  printf("Set mac addr to : %x:%x:%x:%x:%x:%X\r\n",mac_addr[0],mac_addr[1],mac_addr[2],mac_addr[3],mac_addr[4],mac_addr[5]);
  printf("Set ip add to : %i.%i.%i.%i\r\n",ip_addr[0],ip_addr[1],ip_addr[2],ip_addr[3]);
  /*Set the device mac addr*/
  rflpc_eth_set_mac_address(mac_addr);  

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
  
  printf("Set adresse for tx descriptor and status\r\n");
  /*Initialize and sets tx descriptors and status base address*/
  for (i = 0 ; i < NUMBER_PACKET_TX ; i++)
    tx_descriptors[i].packet = tx_buffers[i];  
  rflpc_eth_set_tx_base_addresses ( tx_descriptors, tx_status, NUMBER_PACKET_TX); 
  
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

  send_ping();

  /*Infinite boucle*/
  while(1)
    rflpc_idle;
 
  return 0;
}
