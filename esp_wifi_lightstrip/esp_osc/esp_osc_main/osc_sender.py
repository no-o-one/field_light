
import argparse
import time

from pythonosc import udp_client


if __name__ == "__main__":
  parser = argparse.ArgumentParser()
  parser.add_argument("--ip", default="192.168.1.139",
      help="The ip of the OSC server")
  parser.add_argument("--port", type=int, default=6000,
      help="The port the OSC server is listening on")
  args = parser.parse_args()

  client = udp_client.SimpleUDPClient(args.ip, args.port, 10)

  for x in range(1):
    client.send_message("/strip/preset/1", 0)
    print("sent")
    time.sleep(1)