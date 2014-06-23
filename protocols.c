#include <rflpc17xx/rflpc17xx.h>
#include "protocols.h"

/*Compute the IP header checksum. All fields have to be set to 0 (including the checksum field, size in 16 bits word of data*/
uint16_t get_checksum(uint8_t* data, uint8_t size){
  uint8_t i;
  uint32_t checksum = 0;

  for(i=0 ; i < size ; i++, data+=2)
    checksum += (*data<<8) + *(data+1);

  checksum = (checksum >> 16) + (checksum & 0xffff);
  
  return (~checksum)&0xffff;
} 

#define ETHER_TYPE_IPV4 0
#define ETHER_TYPE_ARP 1

void fill_ethernet_header(uint8_t* buffer, uint8_t dest_mac_addr[6], uint8_t ether_type){
  extern uint8_t mac_addr[6];

  ((ETHERNET_HEADER*)buffer)->dest[0]=dest_mac_addr[0];
  ((ETHERNET_HEADER*)buffer)->dest[1]=dest_mac_addr[1];
  ((ETHERNET_HEADER*)buffer)->dest[2]=dest_mac_addr[2];
  ((ETHERNET_HEADER*)buffer)->dest[3]=dest_mac_addr[3];
  ((ETHERNET_HEADER*)buffer)->dest[4]=dest_mac_addr[4];
  ((ETHERNET_HEADER*)buffer)->dest[5]=dest_mac_addr[5];
  ((ETHERNET_HEADER*)buffer)->source[0]=mac_addr[0];
  ((ETHERNET_HEADER*)buffer)->source[1]=mac_addr[1];
  ((ETHERNET_HEADER*)buffer)->source[2]=mac_addr[2];
  ((ETHERNET_HEADER*)buffer)->source[3]=mac_addr[3];
  ((ETHERNET_HEADER*)buffer)->source[4]=mac_addr[4];
  ((ETHERNET_HEADER*)buffer)->source[5]=mac_addr[5];
  
  switch(ether_type){
  case ETHER_TYPE_IPV4 :
    ((ETHERNET_HEADER*)buffer)->ether_type[0]=0x08;
    ((ETHERNET_HEADER*)buffer)->ether_type[1]=0x00;
    break;

  case ETHER_TYPE_ARP :
    ((ETHERNET_HEADER*)buffer)->ether_type[0]=0x08;
    ((ETHERNET_HEADER*)buffer)->ether_type[1]=0x06;
    break;
  }

}

void ping_request(uint8_t* buffer, uint8_t dest_mac_addr[6], uint8_t dest_ip_addr[4]){
  uint16_t checksum;
  extern uint8_t ip_addr[4];

  fill_ethernet_header(buffer,dest_mac_addr,ETHER_TYPE_IPV4);

  ((IP_HEADER*)(buffer+14))->vers_ihl = 0x45;    
  ((IP_HEADER*)(buffer+14))->service = 0x0;
  ((IP_HEADER*)(buffer+14))->total_length[0] = 0;
  ((IP_HEADER*)(buffer+14))->total_length[1] = 28;
  ((IP_HEADER*)(buffer+14))->ttl = 128;        
  ((IP_HEADER*)(buffer+14))->id[0] = 0;
  ((IP_HEADER*)(buffer+14))->id[1] = 0;
  ((IP_HEADER*)(buffer+14))->flags_fragment_position[0] = 0;
  ((IP_HEADER*)(buffer+14))->flags_fragment_position[1] = 0;
  ((IP_HEADER*)(buffer+14))->protocol = 1;
  ((IP_HEADER*)(buffer+14))->ip_source[0] = ip_addr[0];
  ((IP_HEADER*)(buffer+14))->ip_source[1] = ip_addr[1];
  ((IP_HEADER*)(buffer+14))->ip_source[2] = ip_addr[2];
  ((IP_HEADER*)(buffer+14))->ip_source[3] = ip_addr[3];
  ((IP_HEADER*)(buffer+14))->ip_dest[0] = dest_ip_addr[0];
  ((IP_HEADER*)(buffer+14))->ip_dest[1] = dest_ip_addr[1];
  ((IP_HEADER*)(buffer+14))->ip_dest[2] = dest_ip_addr[2];
  ((IP_HEADER*)(buffer+14))->ip_dest[3] = dest_ip_addr[3];
  ((IP_HEADER*)(buffer+14))->checksum[0] = 0x0;
  ((IP_HEADER*)(buffer+14))->checksum[1] = 0x0;
  checksum = get_checksum((uint8_t*)(buffer+14), (((IP_HEADER*)(buffer+14))->vers_ihl & 0xf)*2);
  ((IP_HEADER*)(buffer+14))->checksum[0] = (checksum>>8) & 0xff;
  ((IP_HEADER*)(buffer+14))->checksum[1] = checksum & 0xff;

        
  ((ICMP_HEADER*)(buffer+34))->type = 8;
  ((ICMP_HEADER*)(buffer+34))->code = 0;
  checksum = get_checksum((uint8_t*)(buffer+34), 4);
  ((ICMP_HEADER*)(buffer+34))->checksum[0] = 0x0;
  ((ICMP_HEADER*)(buffer+34))->checksum[1] = 0x0;
  ((ICMP_HEADER*)(buffer+34))->checksum[0] = (checksum>>8) & 0xff;
  ((ICMP_HEADER*)(buffer+34))->checksum[1] = checksum & 0xff;
}

void pong_request(uint8_t* buffer, uint8_t dest_mac_addr[6], uint8_t dest_ip_addr[4], uint8_t id[2], uint8_t seq[2], const uint8_t* data, uint8_t data_size){
  uint16_t checksum;
  extern uint8_t ip_addr[4];

  fill_ethernet_header(buffer,dest_mac_addr,ETHER_TYPE_IPV4);

  ((IP_HEADER*)(buffer+14))->vers_ihl = 0x45;
  ((IP_HEADER*)(buffer+14))->service = 0;
  ((IP_HEADER*)(buffer+14))->total_length[0] = 0;
  ((IP_HEADER*)(buffer+14))->total_length[1] = 28+data_size;
  ((IP_HEADER*)(buffer+14))->id[0] = 0;
  ((IP_HEADER*)(buffer+14))->id[1] = 0;
  ((IP_HEADER*)(buffer+14))->flags_fragment_position[0] = 0;
  ((IP_HEADER*)(buffer+14))->flags_fragment_position[1] = 0;
  ((IP_HEADER*)(buffer+14))->ttl = 128;        
  ((IP_HEADER*)(buffer+14))->protocol = 1;
  ((IP_HEADER*)(buffer+14))->ip_source[0] = ip_addr[0];
  ((IP_HEADER*)(buffer+14))->ip_source[1] = ip_addr[1];
  ((IP_HEADER*)(buffer+14))->ip_source[2] = ip_addr[2];
  ((IP_HEADER*)(buffer+14))->ip_source[3] = ip_addr[3];
  ((IP_HEADER*)(buffer+14))->ip_dest[0] = dest_ip_addr[0];
  ((IP_HEADER*)(buffer+14))->ip_dest[1] = dest_ip_addr[1];
  ((IP_HEADER*)(buffer+14))->ip_dest[2] = dest_ip_addr[2];
  ((IP_HEADER*)(buffer+14))->ip_dest[3] = dest_ip_addr[3];
  ((IP_HEADER*)(buffer+14))->checksum[0] = 0x0;
  ((IP_HEADER*)(buffer+14))->checksum[1] = 0x0;
  checksum = get_checksum((uint8_t*)(buffer+14), (((IP_HEADER*)(buffer+14))->vers_ihl & 0xf)*2);
  ((IP_HEADER*)(buffer+14))->checksum[0] = (checksum>>8) & 0xff;
  ((IP_HEADER*)(buffer+14))->checksum[1] = checksum & 0xff;
      
  ((ICMP_HEADER*)(buffer+34))->type = 0;
  ((ICMP_HEADER*)(buffer+34))->code = 0;
  ((ICMP_HEADER*)(buffer+34))->id[0] = id[0];
  ((ICMP_HEADER*)(buffer+34))->id[1] = id[1];
  ((ICMP_HEADER*)(buffer+34))->seq_num[0] = seq[0];
  ((ICMP_HEADER*)(buffer+34))->seq_num[1] = seq[1];
  ((ICMP_HEADER*)(buffer+34))->checksum[0] = 0x0;
  ((ICMP_HEADER*)(buffer+34))->checksum[1] = 0x0;
  memcpy(buffer+42,data,data_size);
  checksum = get_checksum((uint8_t*)(buffer+34), 4+data_size);
  ((ICMP_HEADER*)(buffer+34))->checksum[0] = (checksum>>8) & 0xff;
  ((ICMP_HEADER*)(buffer+34))->checksum[1] = checksum & 0xff;
}


void arp_request(uint8_t* buffer, uint8_t dest_mac_addr[6], uint8_t dest_ip_addr[4]){
  extern uint8_t mac_addr[6];
  extern uint8_t ip_addr[4];

  fill_ethernet_header(buffer,dest_mac_addr,ETHER_TYPE_ARP);  

  ((ARP_HEADER*)(buffer+14))->network_type[0] = 0;
  ((ARP_HEADER*)(buffer+14))->network_type[1] = 1; /*ETHERNET*/
  ((ARP_HEADER*)(buffer+14))->protocol_type[0] = 0x08; /*IP*/
  ((ARP_HEADER*)(buffer+14))->protocol_type[1] = 0x00;
  ((ARP_HEADER*)(buffer+14))->hardware_addr_length = 6; /*ETHERNET*/
  ((ARP_HEADER*)(buffer+14))->logical_addr_length = 4; /*IPv4*/
  ((ARP_HEADER*)(buffer+14))->operation[0] = 0;
  ((ARP_HEADER*)(buffer+14))->operation[1] = 2; /* 1->REQUEST || 2->REPLY*/
  
  ((ARP_HEADER*)(buffer+14))->sender_hardware_addr[0] = mac_addr[0];
  ((ARP_HEADER*)(buffer+14))->sender_hardware_addr[1] = mac_addr[1];
  ((ARP_HEADER*)(buffer+14))->sender_hardware_addr[2] = mac_addr[2];
  ((ARP_HEADER*)(buffer+14))->sender_hardware_addr[3] = mac_addr[3];
  ((ARP_HEADER*)(buffer+14))->sender_hardware_addr[4] = mac_addr[4];
  ((ARP_HEADER*)(buffer+14))->sender_hardware_addr[5] = mac_addr[5];
  
  ((ARP_HEADER*)(buffer+14))->sender_logical_addr[0] = ip_addr[0];
  ((ARP_HEADER*)(buffer+14))->sender_logical_addr[1] = ip_addr[1];
  ((ARP_HEADER*)(buffer+14))->sender_logical_addr[2] = ip_addr[2];
  ((ARP_HEADER*)(buffer+14))->sender_logical_addr[3] = ip_addr[3];
  
  ((ARP_HEADER*)(buffer+14))->target_hardware_addr[0] = 0;
  ((ARP_HEADER*)(buffer+14))->target_hardware_addr[1] = 0;
  ((ARP_HEADER*)(buffer+14))->target_hardware_addr[2] = 0;
  ((ARP_HEADER*)(buffer+14))->target_hardware_addr[3] = 0;
  ((ARP_HEADER*)(buffer+14))->target_hardware_addr[4] = 0;
  ((ARP_HEADER*)(buffer+14))->target_hardware_addr[5] = 0;
  
  ((ARP_HEADER*)(buffer+14))->target_logical_addr[0] = dest_ip_addr[0];
  ((ARP_HEADER*)(buffer+14))->target_logical_addr[1] = dest_ip_addr[1];
  ((ARP_HEADER*)(buffer+14))->target_logical_addr[2] = dest_ip_addr[2];
  ((ARP_HEADER*)(buffer+14))->target_logical_addr[3] = dest_ip_addr[3];
}

