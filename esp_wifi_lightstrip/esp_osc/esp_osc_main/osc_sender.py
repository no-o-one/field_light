
import argparse
import time

from pythonosc import udp_client


if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("--ip", default="192.168.0.255",
      help="The ip of the OSC server")
  parser.add_argument("--port", type=int, default=6000,
      help="The port the OSC server is listening on")
  args = parser.parse_args()

  client = udp_client.SimpleUDPClient(args.ip, args.port, 10)

# while(True):
#     client.send_message("/strip/0/preset/1", 0)
#     print("sent1")
#     time.sleep(1)
#     client.send_message("/strip/0/preset/0", 0)
#     print("sent1")
#     time.sleep(1)
    # client.send_message("/strip/preset/1", 0)
    # print("sent1")

client.send_message("/strip/0/preset/0", 0)
# time.sleep(0.1)
# client.send_message("/strip/1/preset/11/1", 0)

