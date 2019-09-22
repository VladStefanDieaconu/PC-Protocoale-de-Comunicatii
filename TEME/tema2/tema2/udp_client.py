__author__ = 'Dorinel Filip'

import socket
import base64
import sys
import random
import time
import json
import os
import argparse
from ipaddress import ip_address
from utils.unpriv_port import unprivileged_port_type, get_unprivileged_port_meta
import textwrap


def setup_parser():
    def get_mode_help():
        return textwrap.dedent(
            '''Specifies the mode used for the load generator as following:
* all_once - send each payload in the list once
* manual - let you choose which message to send next
* random - continuously send random payloads from the list''')

    def read_json_file(parent_parser, arg):
        if not os.path.exists(arg):
            parent_parser.error('The file "{}" does not exist!'.format(arg))
        else:
            with open(arg, 'r') as f:
                return json.load(f)  # return parsed JSON

    parser = argparse.ArgumentParser(formatter_class=argparse.RawTextHelpFormatter,
                                     description='UDP Client for Communication Protocols (2018-2019) Homework #2')

    inputs = parser.add_argument_group('Input')
    inputs.add_argument('--input_file', type=lambda arg: read_json_file(parser, arg), metavar='FILE',
                        default='sample_payloads.json',
                        help='JSON file to read payloads from (default: sample_payloads.json)')

    server_address = parser.add_argument_group('Server Address & Port (required)')
    server_address.add_argument('server_ip', help='Server IP', type=ip_address)
    server_address.add_argument('server_port', help='Server Port', type=unprivileged_port_type)

    source_address = parser.add_argument_group('Source Address')
    source_address.add_argument('--source-address', type=ip_address, default='0.0.0.0',
                                help='IP Address to be bind by UDP client (default: unspecified)')
    source_address.add_argument('--source-port', type=unprivileged_port_type, default=0,
                                metavar=get_unprivileged_port_meta('source-port'),
                                help='UDP port to be used as source for this client (default: random port)')

    mode = parser.add_argument_group('Workload changing parameters')
    mode.add_argument('--mode', default='all_once', type=str, choices=('all_once', 'manual', 'random'),
                      help=get_mode_help())

    load = parser.add_argument_group('Load characteristics')
    load.add_argument('--count', type=int,
                      help='Number of packets to be send (only used for when mode is random, default: infinity)')
    load.add_argument('--delay', help='Wait time (in ms) between two messages (default: 0)', type=int, default=0)

    return parser


def setup_socket(parsed_args):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.bind((str(parsed_args.source_address), parsed_args.source_port))
    return sock


def send_message(sock, message, parsed_args):
    to_send = base64.standard_b64decode(message['payload_base64'])
    sent = sock.sendto(to_send, (str(parsed_args.server_ip), parsed_args.server_port))
    print('Sent ({}/{} bytes) << {} >>'.format(sent, len(to_send), message['description']))
    time.sleep(parsed_args.delay / 1000)


def run_all_once(sock, parsed_args):
    for message in parsed_args.input_file:
        send_message(sock, message, parsed_args)


def run_manual(sock, parsed_args):
    header = 'Please chose one of the following'
    options = '\n'.join(['{}. {}'.format(i, x['description']) for i, x in enumerate(parsed_args.input_file)])
    input_ask = 'Input a int ({} - {}): '.format(0, len(parsed_args.input_file) - 1)
    for_exit_string = 'To end the program, input "exit"'
    prompter_string = '\n'.join((header, options, for_exit_string))
    print(prompter_string)

    while True:
        choice = input(input_ask)
        if choice == 'exit':
            return

        choice = int(choice)
        if (choice >= 0) and (choice < len(parsed_args.input_file)):
            message = parsed_args.input_file[choice]
            send_message(sock, message, parsed_args)
        else:
            print('Maybe try a valid option.')


def run_random(sock, parsed_args):
    n = parsed_args.count
    count = 0
    while (n is None) or (count < n):
        message = random.choice(parsed_args.input_file)
        send_message(sock, message, parsed_args)
        count += 1


def main():
    # Parse arguments
    parser = setup_parser()
    parsed_args = parser.parse_args(sys.argv[1:])
    sock = setup_socket(parsed_args)

    print('Client source PORT: {}'.format(sock.getsockname()[1]))

    if parsed_args.mode == 'all_once':
        print('Running in all_once mode...\n')
        run_all_once(sock, parsed_args)
    elif parsed_args.mode == 'manual':
        print('Running in manual mode...\n')
        run_manual(sock, parsed_args)
    else:
        print('Running in random mode..\n')
        run_random(sock, parsed_args)


if __name__ == '__main__':
    main()
