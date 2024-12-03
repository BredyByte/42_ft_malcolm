# ft_malcolm

> This repository contains a "ft_malcolm" project that is part of the École 42 curriculum.

## Table of Contents
1. [Overview](#overview)
2. [RFC 826 and RFC 7042](#rfc-826-and-rfc-7042)
   - [What is RFC 826?](#what-is-rfc-826)
   - [What is RFC 7042?](#what-is-rfc-7042)
3. [ARP Protocol Explanation](#arp-protocol-explanation)
4. [How ft_malcolm Works](#how-ft_malcolm-works)
   - [Program Arguments](#program-arguments)
   - [Execution Example](#execution-example)
   - [Program Behavior](#program-behavior)
   - [Setting Up Virtual Machines](#setting-up-virtual-machines)
5. [Challenges and Solutions](#challenges-and-solutions)
   - [Ignored ARP Replies](#ignored-arp-replies)
   - [ Send ARP Requests Instead of Replies](#send-arp-requests-instead-of-replies)
6. [Tips, Detection, and Protection](#tips-detection-and-protection)
7. [Useful Commands](#useful-commands)

## Overview
ft_malcolm is a 42 École outer core project focused on networking and cybersecurity. The goal is to develop a program in C that performs MAC address spoofing, allowing manipulation of a victim's network traffic by impersonating critical network devices, such as the gateway, within the **Local Area Network (LAN)**.

This project provides hands-on experience with Layer 2 (Data Link Layer) of the **OSI** model and introduces key networking concepts such as: *Host, Node, Packet, Protocol, IP, MAC, LAN, and ARP*. By working on ft_malcolm, students gain practical knowledge of network packet manipulation, enabling them to understand the inner workings of network communication and security.

## RFC 826 and RFC 7042

To enhance understanding and provide a solid theoretical foundation for ft_malcolm, it's essential to explore relevant **RFCs (Request for Comments)**. RFCs are technical documents and standards published by the **Internet Engineering Task Force (IETF)** to define protocols and technologies foundational to the internet. For instance, RFC 1918 defines private IP address ranges, while RFC 2616 outlines the HTTP/1.1 protocol used for web communication.

### What is RFC 826?

**RFC 826**, titled *Address Resolution Protocol (ARP)*, defines the protocol used to map IP addresses to MAC addresses within a LAN. ARP is essential for communication between devices on the same network, as it allows the system to resolve an IP address into a physical MAC address. Without ARP, devices in a LAN wouldn’t be able to discover or communicate with each other efficiently.

### What is RFC 7042?

**RFC 7042**, titled *IANA Considerations and IETF Protocol and Documentation Usage for IEEE 802 Parameters*, focuses on the standardization and documentation of Ethernet parameters, including MAC address usage and Ethernet frame structures. This RFC ensures consistency and clarity in how MAC addresses and related network information are handled, which is crucial when implementing MAC spoofing techniques like those used in ft_malcolm.

## ARP Protocol Explanation

**ARP (Address Resolution Protocol)** is a fundamental protocol used in LANs to map IP addresses to MAC addresses. It operates at Layer 2 (Data Link Layer) of the OSI model and works as follows:

- **Request:** When a device needs to communicate with another device in the same network, it broadcasts an ARP request packet containing the target IP address.
- **Response:** The device with the matching IP responds with its MAC address, allowing the initiator to establish direct communication.

Below is the structure of an ARP packet, represented in C, including the Ethernet and ARP headers:
```c
// Ethernet header
typedef struct __attribute__((packed)) {
    uint8_t dest_mac[ETH_ALEN];     // Destination MAC address
    uint8_t src_mac[ETH_ALEN];      // Source MAC address
    uint16_t ethertype;             // Protocol type (e.g. 0x0806 for ARP)
} t_ethernet_header;

// ARP header
typedef struct __attribute__((packed)) {
    uint16_t htype;                 // Hardware Type (1 for Ethernet)
    uint16_t ptype;                 // Protocol Type (0x0800 for IPv4)
    uint8_t hlen;                   // MAC Address Length
    uint8_t plen;                   // IP Address Length
    uint16_t operation;             // Operation (1 for ARP Request, 2 for ARP Reply)
    uint8_t sender_mac[ETH_ALEN];   // Sender's MAC address
    uint8_t sender_ip[INET4_LEN];   // Sender's IP Address
    uint8_t target_mac[ETH_ALEN];   // Target's MAC address
    uint8_t target_ip[INET4_LEN];   // Target's IP address
} t_arp_header;
```

In ft_malcolm, ARP spoofing exploits this process by sending fake ARP responses, allowing the attacker to intercept, monitor, or manipulate the victim's network traffic by pretending to be a trusted network device, such as the gateway.

For more detailed information on key networking concepts, please refer to the attached file: **Theory.pdf**.

## How ft_malcolm Works

When I started working on ft_malcolm, I faced a lot of questions: how does the program work, how do you run it, what arguments should be used, how to test it, and what role do virtual machines play in this project? Below are detailed explanations to help understand how the project works and how to use it properly.

### Program Arguments

The program takes four arguments when executed:
1. **source_ip:** The IP address that the victim (target) is looking for. This is the target IP address that the victim sends ARP requests for.
1. **source_mac:** The MAC address the attacker will impersonate in the ARP reply. This is the fake MAC address sent to the victim, which will replace the real MAC address of the device with source_ip.
1. **target_ip:** The victim’s IP address. This is the IP address of the device that sends ARP requests. The program waits for ARP requests from this address and responds with a spoofed ARP reply.
1. **target_mac:** The victim’s MAC address. This is the MAC address of the device sending the ARP request and is used to send the spoofed ARP reply to the correct recipient.

### Execution Example

Imagine a network with two devices:
- **Device A (Victim):** IP `10.0.0.20`, MAC `AA:BB:CC:DD:EE:FF`
- **Device B (Router):** IP `10.0.0.1`, MAC `11:22:33:44:55:66`

You want to spoof the ARP entries on **Device A** to make it believe that the router’s IP (`10.0.0.1`) is associated with your machine’s MAC address (`FF:EE:DD:CC:BB:AA`).

Command execution:
```bash
sudo ./ft_malcolm 10.0.0.1 FF:EE:DD:CC:BB:AA 10.0.0.20 AA:BB:CC:DD:EE:FF
```

### Program Behavior

* **Wait for ARP Requests:** The program waits for Device A to send an ARP request to find the MAC address for `10.0.0.1`. ARP requests are broadcast (put as dest_mac `FF:FF:FF:FF:FF:FF`) messages visible to all devices on the network.
* **Reply with Spoofed Information:** When an ARP request is detected, the program sends a spoofed ARP reply to **Device A**, claiming that the IP address `10.0.0.1` (Router) is associated with your MAC address `FF:EE:DD:CC:BB:AA`.
* **Update ARP Table:** **Device A** updates its ARP table with this new information, associating `10.0.0.1` with your MAC address.
* **Program Exit:** After sending the spoofed ARP reply, the program exits. **Device A** will now route traffic intended for the router to your machine instead.

### Setting Up Virtual Machines
For the ft_malcolm project, you will need to set up two virtual machines: one for the attacker (**malcolm**) and one for the victim (**victim**). In my case, I used two **Debian 11.6.0** virtual machines with the following extensions installed: *vim net-tools iproute2 arping iputils-ping tcpdump*

Both virtual machines will use the **NAT Network** for networking. Here’s how to set it up in **Oracle VM VirtualBox:**
 1. **Create the NAT Network:**
	* Open VirtualBox and go to **File** > **Preferences** > **Network**.
	* Select **NAT Networks**, click **Add** (the green plus sign), and create a new NAT network.
2. **Configure the Network for the Virtual Machines:**
	* For each virtual machine (**malcolm** and **victim**), open the VM settings.
	* Go to the **Network** tab.
	* Under **Attached to**, choose **NAT Network** and select the NAT network you just created.

This configuration will ensure that both machines can communicate with each other while being isolated from the external network.


## Challenges and Solutions

During the development of **ft_malcolm**, I encountered a significant challenge: my ARP replies were consistently ignored by the victim. Despite correctly crafting and sending ARP reply packets, the victim’s system would not update its ARP table with my spoofed MAC address. Here’s what I discovered and how I solved the issue.

### Ignored ARP Replies
When running the program with the following command:
```bash
sudo ./ft_malcolm 192.168.1.38 aa:bb:cc:dd:ee:11 192.168.1.37 51:98:e3:75:ac:4f
```
I monitored ARP traffic using:
```bash
sudo tcpdump -i wlp2s0 -vvv arp src 192.168.1.38
```
The logs showed two ARP replies:
```bash
11:50:35.833082 ARP, Ethernet (len 6), IPv4 (len 4), Reply 192.168.1.38 is-at 08:00:15:a6:f4:e2 (oui Unknown), length 46
11:50:35.835382 ARP, Ethernet (len 6), IPv4 (len 4), Reply 192.168.1.38 is-at aa:bb:cc:dd:11:22 (oui Unknown), length 46
```
The issue? The legitimate ARP reply always arrived before my spoofed one. No matter how fast I sent the spoofed packet—even sending multiple replies in quick succession—it was consistently ignored. After research and experimentation, I discovered that modern systems often ignore unsolicited ARP replies unless they correspond to an existing ARP request.


### Send ARP Requests Instead of Replies
To overcome this, I modified the program to send ARP **requests** instead of **replies**. By setting the operation field in the ARP header to 1 (ARP Request), I achieved the desired effect:
```c
arp_header->operation = htons(1);  // ARP Request instead of Reply
```
Here's what happens:
* When the victim receives an ARP request, it automatically stores the sender's MAC and IP in its ARP cache.
* This behavior ensures that my spoofed MAC is saved in the ARP cache without needing the victim to initiate an ARP request first.

This approach eliminates the need to wait for a ping from the victim and allows continuous updates to the ARP cache.

## Tips, Detection, and Protection
1. **Proxying Traffic to the Gateway:** If you overwrite the victim's ARP cache and they can no longer access the gateway, they will repeatedly send ARP requests to resolve the gateway's MAC. To avoid detection, your program should forward traffic between the victim and the gateway, acting as a proxy. This ensures the victim believes it still has internet access.
1. **Detecting ARP Spoofing:** To detect if you are being ARP-spoofed:<br>
    * Use tools like **nmap** to scan your network and verify if multiple IP addresses share the same MAC address.
    * Implement static ARP entries using the command:<br>
    ```bash
    arp -s <IP-address> <MAC-address>
    ```
    This binds the MAC address to the IP address permanently, preventing ARP cache manipulation.

## Useful Commands
During the project, there were several commands that I used frequently for testing and debugging. Below is a list of useful commands:
* **`arp -n`** - This command shows the current ARP cache, allowing you to view the mapping of IP addresses to MAC addresses.
* **`arp -d 10.0.2.3`** - This command deletes the entry for IP address **10.0.2.3** from the ARP cache, which forces the machine to re-query for the MAC address.
* **`ping -c 1 10.0.3.1`** - This sends a single ping packet to the IP **10.0.3.1**, which helps trigger the ARP request from the victim machine that our program will respond to.
* **`tcpdump -i enp0s3 -vvv arp`** - This command captures ARP (`icmp` flag for capture ICMP  - Type of package that ping sends) packets on the interface **enp0s3** and provides verbose output, which helps in monitoring the ARP traffic on the victim machine.
* **`ifconfig`** - This command shows the IP addresses and MAC addresses of all active network interfaces on the machine, helping to gather network information for configuring the program.
* **`nmap -sn 192.168.1.0/24 | grep "Nmap scan report for\|MAC Address"`** - This command shows IP and MAK of all hosts connected to the same LAN as you.
