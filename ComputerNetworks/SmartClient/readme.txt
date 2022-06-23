SmartClient.py
CSC 361: Assignment 1
Author: Tyler Fowler (V00752565)

    Here is my submission for Assignment 1. Currently it is contained in 
the SmartClient.py file. It is a client program that connects to a web server
and displays some requested information. Below are a list of options for 
test modes. To compile and run, use: 

    "python3 SmartClient.py {URI} [-opt]".

{URI} can be any website URI that matches the following pattern:

    "[http(s)://][www.]hostname[filepath]".

This is the regex string that I came up with to match this pattern:

    "^(https?://)?([\.\w]+)([/\.\w]*)".

If the program is passed a URL with no filepath, it will make a get request
like so:

    "GET https?://{hostname}/ HTTP/1.1\r\n"
    "Host: {hostname}\r\n"
    "\r\n"

Otherwilse, if it is passed a filepath, its get request will look like so:

    "GET https?://{hostname}{filepath} HTTP/1.1\r\n"
    "Host: {hostname}\r\n"
    "\r\n"

Here is a list of options:

    -v: Originally -v was to include debug information. This is now
        included by default.

    -B: Option to print the body of the HTTP response messages.
        This will only work if the debugging -v flag is set as well.
