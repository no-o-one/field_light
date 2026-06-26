import socket
import struct
import time

ARTNET_IP = "255.255.255.255"  # broadcast
ARTNET_PORT = 6454 # < defautl for artnet
UNIVERSE = 0

def make_artnet_packet(universe, dmx_data):
    id = b'Art-Net\x00' # artnet header
    opcode = struct.pack('<H', 0x5000)   # op code 
    version = struct.pack('>H', 14)
    sequence = b'\x00'
    physical = b'\x00'
    uni = struct.pack('<H', universe)
    length = struct.pack('>H', len(dmx_data))
    return id + opcode + version + sequence + physical + uni + length + dmx_data

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
sock.bind(("", 0))

dmx_data = bytes([20,20,0] * 295)  


while True:
    packet = make_artnet_packet(UNIVERSE, dmx_data)
    sock.sendto(packet, (ARTNET_IP, ARTNET_PORT))
    print("packet sent")
    time.sleep(0.1)
