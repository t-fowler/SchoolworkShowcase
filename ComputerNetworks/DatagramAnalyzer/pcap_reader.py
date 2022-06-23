import sys
import struct
import pcap_struct

def read_pcap_file(pcap_file):
    """ This function reads a pcap file and returns a list packets that were pulled from the file. """
    # Read the Global Header.
    gheader = read_global_header(pcap_file)

    num_packets = 0
    packets = []
    while True:
        # Read the Packet Header.
        num_packets += 1
        pheader = read_packet_header(pcap_file)
        if pheader is None: # End of file.
            break

        # Read the Packet Data.
        tup = read_packet_data(pcap_file, pheader.incl_len)
        if tup is None :  # Check if packet is relevant.
            continue

        eth_header, ipheader, udp_header, icmp_header, payload = tup
        new_packet = pcap_struct.packet(pheader, ipheader, udp_header, icmp_header, num_packets, payload)

        if new_packet.IP_header.offset != 0:
            for packet in packets:  # Fragments update the original packet.
                if new_packet.IP_header.id == packet.IP_header.id:
                    packet.add_fragment(new_packet.IP_header.offset, new_packet.timestamp)
                    break
        else:  # New packets are appended to list.
            packets.append(new_packet)

    return packets



def read_global_header(file):
    """ This function parses the global header section of the pcap file. """
    header = pcap_struct.Global_Header()
    buffer = file.read(24)
    tup = struct.unpack('<IHHiIII', buffer)
    
    # magic_number 4 bytes.
    header.set_magic_number(tup[0])

    # version_major/minor 2 bytes each.
    header.set_version(tup[1], tup[2])

    # thiszone 4 bytes
    header.set_zone(tup[3])

    # sigfigs 4 bytes
    header.set_sigfigs(tup[4])

    # snaplen 4 bytes
    header.set_snaplen(tup[5])

    # network 4 bytes
    header.set_link(tup[6])

    return header



def read_packet_header(file):
    """
    This function parses a packet header section from the pcap file.
    It is also set up such that it will determine if the file is empty
    i.e. there are no more packets to parse. Not accounting for improper
    formatting in the pcap file, this should work.
    """
    buff = file.read(16)
    if buff == b'': # End of file return None
        return None
    elif len(buff) < 16:
        exit(1)
    tup = struct.unpack('<IIII', buff)
    
    # ts_sec 4 bytes
    ts_sec = tup[0]

    # ts_usec 4 bytes
    ts_usec = tup[1]

    #incl_len 4 bytes
    incl_len = tup[2]
    
    # orig_len 4 bytes
    orig_len = tup[3]
    
    header = pcap_struct.Packet_Header(ts_sec, ts_usec, incl_len, orig_len)
    return header



def read_packet_data(file, frame_length):
    """ This function parses a packet data section of the pcap file """
    bytes_read = 0
    udp_header = None
    icmp_header = None
    payload = None

    # Read Ethernet Header 14 bytes
    eth_header = file.read(14)  # NOT PROCESSED
    bytes_read = bytes_read + 14

    # Read IPv4 Header *20 bytes usually.
    ipheader = read_ipv4_header(file)
    if ipheader is None:
        bytes_read = bytes_read + 20
        file.read(frame_length - bytes_read) # 14 bytes ethernet and 20 bytes read checking for ip header.
        return None
    bytes_read = bytes_read + ipheader.ip_header_len

    
    if ipheader.offset != 0:                                                # If this packet is a fragment of a previous packet:
        payload = file.read(frame_length - bytes_read)                      # Read the data bytes. Only the IP header information is required.
        return (eth_header, ipheader, udp_header, icmp_header, payload)

    # Read IP Data
    if ipheader.protocol == 1:                                      # If this is an ICMP packet.
        icmp_header = read_icmp_header(file)                        # Read ICMP header
        bytes_read = bytes_read + 8

        # Echo reply message.
        if icmp_header.type == 0:                                       # If message is an echo reply:
            file.read(frame_length - bytes_read)                        # Read the data bytes
            icmp_header.set_group(icmp_header.seq_num)

        # Echo request message.
        elif icmp_header.type == 8:                                     # If message is an echo request:
            file.read(frame_length - bytes_read)                        # Read the data bytes
            icmp_header.set_group(icmp_header.seq_num)

        # Port unreachable message.
        elif icmp_header.type == 3 and icmp_header.code == 3:           # If the message is an unreachable port:
            sub_ipheader = read_ipv4_header(file)                       # Read the request's IP header.
            bytes_read = bytes_read + sub_ipheader.ip_header_len

            if sub_ipheader.protocol == 1:                                  # If the request was an ICMP request:
                sub_icmp_header = read_icmp_header(file)                    # Read the ICMP header.
                bytes_read = bytes_read + 8
                icmp_header.set_group(sub_icmp_header.seq_num)
                payload = (sub_ipheader, sub_icmp_header)

            elif sub_ipheader.protocol == 17:                               # If the request was a UDP request:
                sub_udpheader = read_udp_header(file)                       # Read the UCP header.
                bytes_read = bytes_read + 8
                icmp_header.set_group(sub_udpheader.src_port)
                payload = (sub_ipheader, sub_udpheader)

            file.read(frame_length - bytes_read)                            # Finally, read the data bytes.

        # Time expired message.
        elif icmp_header.type == 11:                                    # If this is a time expired message:
            sub_ipheader = read_ipv4_header(file)                       # Read the request's IP header.
            bytes_read = bytes_read + sub_ipheader.ip_header_len
            
            if sub_ipheader.protocol == 1:                                  # If request was an ICMP request:
                sub_icmp_header = read_icmp_header(file)                    # Read the request's ICMP header.
                bytes_read = bytes_read + 8
                icmp_header.set_group(sub_icmp_header.seq_num)
                payload = (sub_ipheader, sub_icmp_header)

            elif sub_ipheader.protocol == 17:                               # If request was a UDP request:
                sub_udpheader = read_udp_header(file)                       # Read the UDP header.
                bytes_read = bytes_read + 8
                icmp_header.set_group(sub_udpheader.src_port)
                payload = (sub_ipheader, sub_udpheader)

            file.read(frame_length - bytes_read)                            # Finally, read the data bytes.
        
        else:
            return None
            
    elif ipheader.protocol == 17:                                   # If this is a UDP packet:
        udp_header = read_udp_header(file)                          # Read the UDP header.
        bytes_read = bytes_read + 8
        file.read(frame_length - bytes_read)                        # Read the data bytes.
        if not (33434 <= udp_header.dst_port <= 33529):               # Filter unwanted udp segments.
            return None
        
    return (eth_header, ipheader, udp_header, icmp_header, payload)



def read_ipv4_header(file):
    """
    This function reads an ipv4 header from a pcap file. Some of the code is
    sourced from the provided basic_structures.py and modified. 
    """
    ipheader = pcap_struct.IP_Header()
    tup = struct.unpack('>BBHHHBBHBBBBBBBB', file.read(20))

    # IP Version Number 4 bits
    # IP Header Length 4 bits
    ipv = (tup[0] & 0xF0) >> 4  # Upper nibble.
    ihl = tup[0] & 0x0F  # Lower nibble. In unit of 4 bytes.
    ipheader.set_header_len(ihl * 4)
    
    # Type of Service 8 bits
    tos = tup[1]

    # Total Length 16 bits
    total_length = tup[2]
    ipheader.set_total_len(total_length)

    # Identification 16 bits
    id = tup[3]
    ipheader.set_id(id)

    # Flags 4 bits
    # Fragment Offset 12 bits
    flag = (tup[4] & 0xF000) >> 12  # First nibble.
    foffset = tup[4] & 0x0FFF  # Second nibble and lower byte.
    ipheader.set_frag(flag, foffset)

    # Time to Live 8 bits
    # Protocol 8 bits
    # Header Checksum 16 bits
    ttl = tup[5]
    protocol = tup[6]
    checksum = tup[7]
    ipheader.set_protocol(protocol)
    ipheader.set_ttl(ttl)

    if not (ipheader.protocol == 1 or ipheader.protocol == 17):
        return None

    # Source IP Address 32 bits
    # Destination IP Adress 32 bits
    src_addr = (tup[8], tup[9], tup[10], tup[11])
    dest_addr = (tup[12], tup[13], tup[14], tup[15])

    src_ip = str(src_addr[0])+'.'+str(src_addr[1])+'.'+str(src_addr[2])+'.'+str(src_addr[3])
    dest_ip = str(dest_addr[0])+'.'+str(dest_addr[1])+'.'+str(dest_addr[2])+'.'+str(dest_addr[3])

    ipheader.set_ip(src_ip, dest_ip)

    # Read optional bytes.
    optbytes = ihl * 4 - 20
    if optbytes:
        opt = file.read(optbytes)

    return ipheader



def read_udp_header(file):
    """ Reads a udp header from a pcap file. """
    # src port: 2 bytes
    # dst port: 2 bytes
    # l: 2 bytes
    # checksum: 2 bytes
    src, dst, l, sum = struct.unpack('>HHHH', file.read(8))
    return pcap_struct.UDP_Header(src, dst, l, sum)



def read_icmp_header(file):
    """
    Reads an icmp header from the data portion of the IP datagram.
    For discovering the type of icmp message.
    """
    # Type: 1 byte  
    # Code: 1 byte
    # Checksum: 2 bytes
    # ID: 2 bytes
    # Seq_num: 2 bytes
    icmp_type, code, checksum, ID, seq_num = struct.unpack('>BBHHH', file.read(8))
    
    # Only Echo requests and replies have non null ID and checksum fields.
    if icmp_type != 0 and icmp_type != 8:
        header = pcap_struct.ICMP_Header(icmp_type, code, checksum)
    else:
        header = pcap_struct.ICMP_Header(icmp_type, code, checksum, ID, seq_num)

    return header