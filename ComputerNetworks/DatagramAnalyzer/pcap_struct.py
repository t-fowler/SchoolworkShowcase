# CSC 361: Assignment 3
# Author: Tyler Fowler
# ID: V00752565
import struct

FRACTIONAL_TIME_FACTOR = None

class Global_Header:
    """Representation of the global header for the pcap file."""
    magic_number = 0
    version_major = 0
    version_minor = 0
    thiszone = 0
    sigfigs = 0
    snaplen = 0
    network = 0

    def __init__(self):
        self.magic_number = 0
        self.version_major = 0
        self.version_minor = 0
        self.thiszone = 0
        self.sigfigs = 0
        self.snaplen = 0
        self.link = 0
        self.endianness = 0

    def set_magic_number(self, mag_num):
        """
        This function sets the magic number for the pcap_file. It is assumed
        that test files for this assignment will have the standard magic
        number '0xa1b2c3d4'.
        """
        global FRACTIONAL_TIME_FACTOR
        if mag_num == 0xa1b2c3d4:
            self.magic_number = mag_num
            FRACTIONAL_TIME_FACTOR = 0.000001
        elif mag_num == 0xa1b23c4d:
            self.magic_number = mag_num
            FRACTIONAL_TIME_FACTOR = 0.000000001
        else:
            print(f'Magic number {mag_num} not recognized. Cannot read pcap file.')
            exit()
    
    def set_version(self, v_maj, v_min):
        """
        This function sets the version of the pcap file. it is passed as an int
        parameter and needs to be separated by lower/upper byte.
        """
        self.version_major = v_maj
        self.version_minor = v_min
    
    def set_zone(self, zone):
        """
        Sets the time zone of the pcap file. the time zone is represented by an
        offset in seconds from the GMT time zone.
        """
        self.thiszone = zone
    
    def set_sigfigs(self, figs):
        """ Sets the sigfigs value for the pcap file. """
        self.sigfigs = figs

    def set_snaplen(self, len):
        """ Sets the snaplen value for the pcap file. """
        self.snaplen = len

    def set_link(self, link):
        """
        Sets the network standard for the pcap file. For this assignment,
        it is assumed that the network is an ethernet connection.
        """
        self.link = link
        if self.link != 1:
            print(f'Link layer type [{self.link}] not supported. Only ethernet is supported.')
            exit()
    
    def print(self):
        """Print extracted data from the global header."""
        print(f'Magic Number: {self.magic_number}')
        print(f'Version Major: {self.version_major}')
        print(f'Version Minor: {self.version_minor}')
        print(f'ThisZone: {self.thiszone}')
        print(f'SigFigs: {self.sigfigs}')
        print(f'Snapshot Length: {self.snaplen}')
        print(f'Link Layer Type: {self.link}')



class IP_Header:
    """ Representation of the IP header for the packet. """
    src_ip = None
    dst_ip = None
    id = None
    flags = None
    offset = None
    ttl = None
    ip_header_len = None
    total_len = None
    protocol = None
    
    def __init__(self):
        self.src_ip = None
        self.dst_ip = None
        self.id = None
        self.flags = 0
        self.offset = 0
        self.ttl = 0
        self.ip_header_len = 0
        self.total_len = 0
        self.protocol = 0
    
    def set_ip(self,src_ip,dst_ip):
        """Sets the source and destination ip address. """
        self.src_ip = src_ip
        self.dst_ip = dst_ip
    
    def set_id(self, id):
        """ Sets the identification number in the ip header. """
        self.id = id

    def set_ttl(self, ttl):
        """ Sets the time to live value for the IP header. """
        self.ttl = ttl
    
    def set_frag(self, flags, offset):
        """ Sets the fragmentation fields in the IP header. """
        self.flags = flags
        self.offset = offset

    def set_header_len(self,length):
        """ Sets the header length for the ip header. """
        self.ip_header_len = length
    
    def set_total_len(self, length):
        """ Sets the total length field for the ip header. """
        self.total_len = length    

    def set_protocol(self, protocol):
        """ Sets the protocol field for the ip header. """
        self.protocol = protocol

    def print(self):
        """ For debugging, prints the data in the ip header. """
        print(f'Source IP: {self.src_ip}')
        print(f'Destination IP: {self.dst_ip}')
        print(f'IP Header Length: {self.ip_header_len}')
        print(f'Total Length: {self.total_len}')
 


class TCP_Header:
    """ Representation of the tcp headers in the pcap file. """
    src_port = 0
    dst_port = 0
    seq_num = 0
    ack_num = 0
    data_offset = 0
    flags = {}
    window_size =0
    checksum = 0
    ugp = 0
    
    def __init__(self):
        self.src_port = 0
        self.dst_port = 0
        self.seq_num = 0
        self.ack_num = 0
        self.data_offset = 0
        self.flags = {}
        self.window_size =0
        self.checksum = 0
        self.ugp = 0
    
    def set_src_port(self, src):
        """ Sets the src port field for the tcp header. """
        self.src_port = src
        
    def set_dst_port(self,dst):
        """ Sets the dst port field for the tcp header. """
        self.dst_port = dst
        
    def set_seq_num(self,seq):
        """ Sets the sequence number field for the tcp header. """
        self.seq_num = seq
        
    def set_ack_num(self,ack):
        """ Sets the acknowledgment number field for the tcp header. """
        self.ack_num = ack
        
    def set_data_offset(self,data_offset):
        """ Sets the data offset field for the tcp header. """
        self.data_offset = data_offset
        
    def set_flags(self, ack, rst, syn, fin):
        """ Sets the flags for the tcp header. """
        self.flags["ACK"] = ack
        self.flags["RST"] = rst
        self.flags["SYN"] = syn
        self.flags["FIN"] = fin
    
    def set_win_size(self,size):
        """ Sets the window size field for the tcp header. """
        self.window_size = size

    def set_checksum(self, checksum):
        """ Sets the checksum field for the tcp header. """
        self.checksum = checksum

    def print(self):
        """ For debugging, print the data from the tcp header. """
        print(f'Source Port: {self.src_port}')
        print(f'Destination Port: {self.dst_port}')
        print(f'Sequence Number: {self.seq_num}')
        print(f'Acknowledgment Number: {self.ack_num}')
        print(f'Data Offset: {self.data_offset}')
        print(f'Flags: {self.flags}')
        print(f'Window Size: {self.window_size}')
        print(f'Checksum: {self.checksum}')
        print(f'Urgent Pointer: {self.ugp}')
    
    def relative_seq_num(self,orig_num):
        """ Get the relative sequence number of the packet. """
        if(self.seq_num>=orig_num):
            relative_seq = self.seq_num - orig_num
            self.set_seq_num(relative_seq)
        #print(self.seq_num)
        
    def relative_ack_num(self,orig_num):
        """ Get the relative aknowledgment number of the packet. """
        if(self.ack_num>=orig_num):
            relative_ack = self.ack_num-orig_num+1
            self.set_ack_num(relative_ack)
   


class UDP_Header:
    src_port = None
    dst_port = None
    length = 0
    checksum = 0

    def __init__(self, src, dst, length, checksum):
        self.src_port = src
        self.dst_port = dst
        self.length = length
        self.checksum = checksum



class ICMP_Header:
    type = None
    code = None
    checksum = None
    ID = None
    seq_num = None
    group = None

    def __init__(self, type, code, checksum, ID=None, seq_num=None):
        """ Constructor for an ICMP datagram. """
        self.type = type
        self.code = code
        self.checksum = checksum
        self.ID = ID
        self.seq_num = seq_num

        if type == 0:  # Echo Reply message
            pass
        elif type == 8:  # Echo Ping message
            pass
        elif type == 3 and code == 3:
            pass
        elif type == 11:  # Time Exceeded Message
            pass
        else:
            print(f'ICMP Datagram with type={type} is not recognized.')
            return None
    
    def set_group(self, group):
        self.group = group



class Packet_Header:
    """ Representation of the packet header sections of the pcap file. """
    ts_sec = 0
    ts_usec = 0
    incl_len = 0
    orig_len = 0

    def __init__(self, sec, usec, incl, orig):
        """ Initialize the packet header with the given values. """
        self.ts_sec = sec
        self.ts_usec = usec
        self.incl_len = incl
        self.orig_len = orig

    def print(self):
        """ For debugging, print the values of the packet header. """
        print(f'Timestamp (sec): {self.ts_sec}')
        print(f'Timestamp (usec): {self.ts_usec}')
        print(f'Included Length: {self.incl_len}')
        print(f'Original Length: {self.orig_len}')



class packet():
    """ Representation of a packet in the pcap file. """
    
    #pcap_hd_info = None
    P_header = None
    IP_header = None
    UDP_header = None
    ICMP_header = None
    payload = None
    num_fragments = 0
    fragment_offsets = []
    fragment_timestamps = []
    timestamp = 0
    packet_No = 0
    num_bytes = 0
    buffer = None
    
    def __init__(self, pheader, ipheader, udpheader, icmpheader, num_packet, payload=None):
        """ Create a packet with the given packet, ip, and tcp headers. """
        self.P_header = pheader
        self.IP_header = ipheader
        self.UDP_header = udpheader
        self.ICMP_header = icmpheader
        self.payload = payload
        self.num_fragments = 1
        self.fragment_offsets = [0]
        self.timestamp = pheader.ts_sec + pheader.ts_usec * FRACTIONAL_TIME_FACTOR
        self.fragment_timestamps = [self.timestamp]
        self.packet_No = num_packet
        self.num_bytes = pheader.orig_len
        self.buffer = None

    def is_udp(self):
        """ Returns true if the packet is a udp packet. """
        return self.IP_header.protocol == 17

    def is_echo(self):
        """ Returns true if the packet is an icmp echo request. """
        if self.ICMP_header is not None:
            return self.ICMP_header.type == 8
        else:
            return True

    def add_fragment(self, offset, timestamp):
        """ This function is for keeping track of fragment information of packets. """
        self.num_fragments += 1
        if offset in self.fragment_offsets:
            print('Error duplicate offset.')
            exit()
        self.fragment_offsets.append(offset)
        self.fragment_timestamps.append(timestamp)

    def get_group_value(self):
        if self.is_udp():
            return self.UDP_header.src_port
        else:
            return self.ICMP_header.group
    
    def print(self):
        print(f'packet_no: {self.packet_No} time: {self.timestamp} src: {self.IP_header.src_ip} dst: {self.IP_header.dst_ip}, protocol: {self.IP_header.protocol}')



class ICMP_Conversation():
    """ Representation of a single tracerount request and the icmp responses that are returned. """
    orig_packet= None
    resp_packet = None

    def __init__(self, orig_packet):
        self.orig_packet = orig_packet
        self.resp_packet = None

    def add_response(self, response):
        if self.resp_packet is not None:
            print('Error: Second response to same ping.')
            exit()
        self.resp_packet = response

    def get_group_value(self):
        return self.orig_packet.get_group_value()