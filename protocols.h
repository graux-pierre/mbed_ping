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

#endif
