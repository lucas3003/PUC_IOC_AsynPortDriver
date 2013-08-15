from ServerSock import ServerSock as Super
from random import *

class BPM(Super):
	def __init__(self):
		Super('127.0.0.1', 6543, self.handler)

	def handler(self, clientsock, addr):
		while 1:
			data = clientsock.recv(16391)

			if not data:
				print "Nothing received"
				break

			for i in range(0,len(data)):
				print"Received on byte %d: %d" %(i, ord(data[i]))

			print "End of data"

			command = ord(data[0])

			if command == 0x02:
				#Simulate "list var" command
				packet = [0x03, 0x06, 0x83, 0x03, 0x03, 0x03, 0x03, 0x83]

			if command == 0x04:
				#Simulate "list group var" command
				packet = [0x05, 0x00]

			if command == 0x08:
				#Simulate "list curves" command
				packet = [0x09, 0x00]


			if command == 0x10:
				#Simulate response of 8 bytes (64 bits)
				packet = [0x11, 0x08, randint(0,255), randint(0,255), randint(0,255), randint(0,255), randint(0,255), randint(0,255), randint(0,255), randint(0,255)]

			if command == 0x20:
				#Simulate a sucesfull write
				packet = [0xE0, 0x00]

			response = bytearray(packet)
			clientsock.send(response)
			print "Response send"

sock = BPM()