import sys
import copy
import math
import pcap_struct
import pcap_reader

def unique(ls):
    """ This function returns a list containing only the unique values in the parameter list. """
    unique_list = []
    for item in ls:
        if item not in unique_list:
            unique_list.append(item)
    return unique_list



def gen_output(trace, packets):
    """ This function generates the output for requirement 1 of assignment 3. """
    replies = [packet for packet in packets if not(packet.is_udp() or packet.is_echo())]    # List of ICMP reply messages from the traceroute.
    hops = unique([reply.IP_header.src_ip for reply in replies])                            # List of all the IP addresses that respond to the trace requests.

    src_node_ip = trace[0].orig_packet.IP_header.src_ip                                     # Source ip is the ip sending the original packets.
    dst_node_ip = trace[0].orig_packet.IP_header.dst_ip                                     # Destination ip is the ip the source is sending to.
    rtts = [0 for hop in hops]                                                              # List of the average round trip times for each trace request/reply message.
    stan_devs = [0 for hop in hops]                                                         # List of the standard deviations of the average rtts.
    num_frags = [0 for hop in hops]                                                         # List of the number of fragments sent by the trace requests.
    inter_node_ips = copy.copy(hops)[:-1]                                                   # List of the intermediate routers in the trace. (All responding routers except for the destination router).
    protocols = [con.orig_packet.IP_header.protocol for con in trace]                       # List of protocols used in sending trace requests.             
    protocols.extend([con.resp_packet.IP_header.protocol for con in trace])                 # List of protocols used in the trace replies.

    # Print source and destination ip addresses.
    print(f'The IP address of the source node: {src_node_ip}')  
    print(f'The IP address of ultimate destination node: {dst_node_ip}')

    # Print the intermediate router ip addresses.
    print('The IP addresses of the intermediate destination nodes:')
    for i, router in enumerate(inter_node_ips):
        print(f'\trouter {i+1}: {router}' + ('.' if i == len(inter_node_ips) - 1 else ','))
    print('')
    
    # Print the protocols used in the traceroute.
    print('The values in the protocol field of IP headers:')
    if 1 in protocols:
        print('1: ICMP ')
    if 17 in protocols:
        print('17: UDP')
    print('')
    print('')

    # Print the fragment information of each trace request.
    for message in trace:
        id = message.get_group_value()
        num_fragments = message.orig_packet.num_fragments - 1
        last_offset = message.orig_packet.fragment_offsets[-1]
        print(f'The number of fragments created from the original datagram {id} is: {num_fragments}')
        print(f'The offset of the last fragment is: {last_offset}')
        print ('')

    # Add up all the rtt values of each fragment for each router.
    for message in trace:
        hop = message.resp_packet.IP_header.src_ip
        timestamps = message.orig_packet.fragment_timestamps
        idx = hops.index(hop)

        for time in timestamps:
            num_frags[idx] += 1
            rtts[idx] += (message.resp_packet.timestamp - time) * 1000

    # Divde by the number of fragments sen tto that router to get the average rtt.
    for idx, ip in enumerate(hops):
        rtts[idx] /= num_frags[idx]

    # Next two loops calculate the standard deviations in average rtt for each router.
    for message in trace:
        hop = message.resp_packet.IP_header.src_ip
        timestamps = message.orig_packet.fragment_timestamps
        idx = hops.index(hop)

        for time in timestamps:
            stan_devs[idx] += ((message.resp_packet.timestamp - time)* 1000 - rtts[idx]) ** 2
    
    for idx, ip in enumerate(hops):
        stan_devs[idx] /= num_frags[idx]
    stan_devs = [math.sqrt(sd) for sd in stan_devs]

    # Print the rtt data for each router.
    for idx, hop in enumerate(hops):
        print(f'The avg RTT between {src_node_ip} and {hop} is: {round(rtts[idx], 6)} ms, the s.d. is: {round(stan_devs[idx], 6)} ms')



def main(opt):
    # Open pcap file
    if not opt:
        print('Please provide a filename as a fuction argument.')
        exit()

    pcap_file = None
    try:
        pcap_file = open(opt[0], 'rb')
    except:
        print(f'Error opening file: {opt[0]}. Make sure the name is correct.')
        exit()        
    
    packets = pcap_reader.read_pcap_file(pcap_file)
    traceroute = []
    for packet in packets:

        # UDP and ICMP echo requests are original datagrams, else it is a response.
        if packet.is_udp() or packet.is_echo():
            traceroute.append(pcap_struct.ICMP_Conversation(packet))
        else:
            for con in traceroute:
                if con.orig_packet.get_group_value() == packet.get_group_value():
                    con.resp_packet = packet
                    break
    
    incomplete = [con for con in traceroute if con.resp_packet is None]
    traceroute = [con for con in traceroute if con not in incomplete]

    #if incomplete:
    #    print('Incomplete messages.')
    #    for con in incomplete:
    #        con.orig_packet.print()

    gen_output(traceroute, packets)
        


if __name__ == "__main__":
    opt = []
    if len(sys.argv) > 1:
        opt.extend(sys.argv[1:])
    main(opt)
