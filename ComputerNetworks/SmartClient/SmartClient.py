import sys
import re
import socket
import ssl

DEBUG = 'ON'
KEY = {'HTTPS': 'None', 'HTTP/2': 'None', 'HTTP/1.1': 'None'}

def parse_URI(input):
    """
    This function matches input against an internally defined regex string.
    If there is a match, it returns the hostname and filepath as a tuple.
    If there is no match, the program fails loudly.
    """
    # Set up regex.
    URI_REGEX = r'^(https?://)?([\w\.]+)([/\.\w]*)'
    URI_match = re.match(URI_REGEX, input)
    if URI_match is None:
        print(f'{input} does not match URI. Must be of the form')
        print(f'[http(s)://]hostname[filepath]')
        exit()
    
    # Extract hostname and filepath.
    hostname = URI_match.group(2)
    filepath = URI_match.group(3)
    if filepath == '':
        filepath = '/'
    
    # Print debugging information
    if DEBUG == 'ON':
        print('[URI MATCH]')
        print(f'Hostname: {hostname}')
        print(f'Filepath: {filepath}\n')

    return (hostname, filepath)

def read_http_response(message):
    """
    This method interprets an http response message. It returns
    a 3 tuple with the status line, header fields and entity body
    as its elements. 
    """
    # Extract the status line
    message_p = message.partition('\r\n')
    status = message_p[0]
    if message_p[2] == '':
        print('Error: No response from server.')

    # Extract the header lines
    headers = []
    while message_p[2].find('\r\n') > 0:
        message_p = message_p[2].partition('\r\n')
        headers.append(message_p[0])
    
    # Extract the body
    body = message_p[2][2:]

    return (status, headers, body)

def cookie_key(headerlist):
    """
    This helper method takes a list of headers from and HTTP response
    message and returns the cookie data as required output.
    """
    cookielist = [c[12:] for c in headerlist if 'Set-Cookie:' in c]
    res = ''
    
    for cookie in cookielist:
        cookie = cookie.split('; ')
        #Extract cookie name from the form <Name>=<value>
        res = res + 'Cookie Name: ' + cookie[0].split('=')[0]
        cookie = cookie[1:]
        # Extract required attributes from the form <attribute>=<value>
        for attr in cookie:
            attr = attr.split('=')
            if attr[0].lower() == 'expires':
                res = res + ', expires: ' + attr[1]
            elif attr[0].lower() == 'domain':
                res = res + ', Domain_Name: ' + attr[1]
        res = res + '\n'
    return res[0:-1]

def check_http1(hostname, filepath, http_version):
    """
    Sends and http request to {hostname} request a response with {filepath}.
    This function returns HTTP response message (or None if a message was unable
    to be sent) as a 3-tuple containing the status line, header fields and message body.
    """
    global DEBUG, KEY
    # Establish connection to hostname.
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((hostname, 80))
    except socket.gaierror:
        # Error with URI
        print(f'Error: Cannot resolve {hostname} to an IP address.')
        print('Possibly a regex matching bug:')
        print(f'{hostname} should match "[http(s)://][www.]hostname[filepath]"')
        exit()

    # Send a request.
    req = f'GET http://{hostname}{filepath} {http_version}\r\n'  \
        + f'Host: {hostname}\r\n'                                \
        + '\r\n'
    if DEBUG == 'ON':
            print(f'[Sending {http_version} Request Un-Encrypted]')
            print(req[0:-2])
    
    sock.send(req.encode())
    resp = sock.recv(8192)
    
    # Read response, update answer key, return response tuple.
    resp_tup = read_http_response(resp.decode())
    status_code = resp_tup[0][9:12]
    if status_code == '1' or status_code == '2' or status_code == '3' or http_version.lower() in resp_tup[0].lower():
        KEY.update({'HTTP/1.1': 'yes'})
    else:
        KEY.update({'HTTP/1.1': 'no'})

    # Close socket and return.
    sock.close()
    return resp_tup
    
def check_https(hostname, filepath, http_version):
    """
    Sends and http request over ssl to {hostname} requesting a response with {filepath}.
    This function returns the decoded HTTP response message (or None if a request
    was unable to be sent) as a 3-tuple containing the status line, header fields and
    message body as elements.
    """
    global DEBUG, KEY
    resp_tup = None
    
    # Establish connection to hostname.
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    try:
        sock.connect((hostname, 443))
    except socket.gaierror:
        # Error with URI
        print(f'Error: Cannot resolve {hostname} to an IP address.')
        print('Possibly a regex matching bug:')
        print(f'{hostname} should match "[http(s)://][www.]hostname[filepath]"')
        exit()
    
    # Attempt to wrap socket with SSL.
    ssl_ctx = ssl.create_default_context(purpose=ssl.Purpose.CLIENT_AUTH)
    ssl_ctx.set_alpn_protocols([http_version])
    try:    
        ssock = ssl_ctx.wrap_socket(sock, server_hostname=hostname)
    except ssl.CertificateError:
        # Does not support HTTPS and therefore shouldn't support HTTP/2
        KEY.update({'HTTPS': 'no', 'HTTP/2': 'no'})
        print('[SSL CertificateError. Website does not support HTTPS]\n')
        return
    except:
        KEY.update({'HTTPS': 'no', 'HTTP/2': 'no'})
        print('SSL error. Trying un-encrypted message.\n')
        return

    selected_version = ssock.selected_alpn_protocol()

    if DEBUG == 'ON':
        if selected_version == http_version:
            print('[ALPN Version Selected]')
            print(http_version + ' is supported over SSL with ALPN.')
        else:
            print('[ALPN Selected Version does not match requested version]')
            print(http_version + ' is not supported over SSL with ALPN.')
        print('')
        
    if http_version == 'h2':
        # TO DO: if time, implement HTTP/2 messaging.
        if selected_version == 'h2':
            KEY.update({'HTTPS': 'yes', 'HTTP/2': 'yes'})
        else:
            KEY.update({'HTTP/2': 'no'})

    elif http_version == 'http/1.1':
        # Send an http/1.1 request
        req = f'GET https://{hostname}{filepath} HTTP/1.1\r\n'      \
            + f'Host: {hostname}\r\n'                               \
            + '\r\n'
        if DEBUG == 'ON':
            print('[Sending HTTP/1.1 Request over SSL]')
            print(req[0:-2])
        
        ssock.send(req.encode())
        resp = ssock.recv(8192)

        # Read the response, update answer key.
        resp_tup = read_http_response(resp.decode())
        status_code = resp_tup[0][9:12]
        if status_code[0] == '1' or status_code[0] == '2' or status_code[0] == '3' or http_version.lower() in resp_tup[0].lower():
            KEY.update({'HTTPS': 'yes', 'HTTP/1.1': 'yes'})
        else:
            if KEY['HTTPS'] == 'None':
                KEY.update({'HTTPS': 'no'})
            KEY.update({'HTTP/1.1': 'no'})
   
    else:
        # Some kind of error.
        print('Program Error: HTTP version passed to check_https() not correct')
        exit()

    # Close socket and return
    ssock.close()
    sock.close()
    return resp_tup

def main():
    """
    The SmartClient program is designed to determine if a website supports
    HTTP/1.1, Http/2.0 and HTTPS. It also provides a small amount of cookie
    information sent from those web servers.
    """
    global DEBUG, KEY
    if '-v' in sys.argv:
        DEBUG = 'ON'
    
    # Parse input.
    hostname, filepath = parse_URI(sys.argv[1])
    status = None
    headers = None
    body = None
    cookielist = ''

    # HTTP/2 over SSL
    message = check_https(hostname, filepath, 'h2')

    # HTTP/1.1 over SSL
    if KEY['HTTPS'] != 'no':
        message = check_https(hostname, filepath, 'http/1.1')
        if message is not None:
            status, headers, body = message
            cookielist = cookie_key(headers)
    
    # HTTP/1.1 Un-encrypted.
    if KEY['HTTPS'] == 'no':
        message = check_http1(hostname, filepath, 'HTTP/1.1')
        if message is not None:
            status, headers, body = message
            cookielist = cookie_key(headers)

    # Print response message information
    if DEBUG == 'ON':
        if message is not None:
            print('[Response Received]')
            if status is not None:
                print(status)
                print('')
            if headers:
                print('[Headers]')
                for header in headers:
                    print(header)
                print('')
            if '-B' in sys.argv:
                print('[Body]')
                print(body)
                print('')
    
    # Output answer key
    print(f'website: {hostname}')
    print('1. Supports https: '     + KEY['HTTPS'])
    print('2. Supports http1.1: '   + KEY['HTTP/1.1'])
    print('3. Supports http2: '     + KEY['HTTP/2'])
    if cookielist:
        print('4. List of cookies:\n' + cookielist)

if __name__ == '__main__':
    main()