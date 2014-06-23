#ifndef HEADER_PROTOCOLS
#define HEADER_PROTOCOLS

struct ethernet_header {
  uint8_t dest[6];
  uint8_t source[6];
  uint8_t ether_type[2];
  uint8_t fields_8021Q[2];
  uint8_t ether_type_8021Q[2];
};
typedef struct ethernet_header ETHERNET_HEADER;

struct arp_header {
  uint8_t network_type[2];
  uint8_t protocol_type[2];
  uint8_t hardware_addr_length;
  uint8_t logical_addr_length;
  uint8_t operation[2];
  uint8_t sender_hardware_addr[6];
  uint8_t sender_logical_addr[4];
  uint8_t target_hardware_addr[6];
  uint8_t target_logical_addr[4];
};
typedef struct arp_header ARP_HEADER;

struct ip_header {
  uint8_t vers_ihl;
  uint8_t service;
  uint8_t total_length[2];
  uint8_t id[2];
  uint8_t flags_fragment_position[2];
  uint8_t ttl;
  uint8_t protocol;
  uint8_t checksum[2];
  uint8_t ip_source[4];
  uint8_t ip_dest[4];
};
typedef struct ip_header IP_HEADER;


struct icmp_header {
  uint8_t type;
  uint8_t code;
  uint8_t checksum[2];
  uint8_t id[2];
  uint8_t seq_num[2];
};
typedef struct icmp_header ICMP_HEADER;

struct udp_header {
  uint8_t source_port[2];
  uint8_t dest_port[2];
  uint8_t length[2];
  uint8_t checksum[2];
};
typedef struct udp_header UDP_HEADER;


struct tcp_header {
  uint8_t source_port[2];
  uint8_t dest_port[2];
  uint8_t seq[4];
  uint8_t ack_number[4];
  uint8_t flags[2];
  uint8_t windows[2];
  uint8_t checksum[2];
  uint8_t urg_opinter[2];
};
typedef struct tcp_header TCP_HEADER;


/*Compute the IP header checksum. All fields have to be set to 0 (including the checksum field, size in 16 bits word of data*/
uint16_t get_checksum(uint8_t* data, uint8_t size);

void ping_request(uint8_t* buffer, uint8_t dest_mac_addr[6], uint8_t dest_ip_addr[4]);

void arp_request(uint8_t* buffer, uint8_t dest_mac_addr[6], uint8_t dest_ip_addr[4], uint8_t source_mac_addr[6], uint8_t source_ip_addr[4]);

void pong_request(uint8_t* buffer, uint8_t dest_mac_addr[6], uint8_t dest_ip_addr[4], uint8_t id[2], uint8_t seq[2],const uint8_t* data, uint8_t data_size);

#endif
