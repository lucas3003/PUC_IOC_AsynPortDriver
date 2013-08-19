from ServerSock import ServerSock as Super
from random import *

class PUC(Super):

	def __init__(self):
		Super('127.0.0.1', 6791, self.handler)

	def checksum(self, packet):
		cs = 0x00

		for i in range(len(packet)):
			cs = cs + packet[i]

		cs = 0x100 - (cs&0xFF)
		return cs


	def handler(self, clientsock, addr):
		#Simulate 0x05 address
		self.address = 0x05;
		while 1:
			data = clientsock.recv(16391)

			if not data:
				print "Nothing received"
				break

			for i in range(0,len(data)):
				print"Received on byte %d: %d" %(i, ord(data[i]))

			print "End of data"

			command = ord(data[2])

			if command == 0x02:
				#Simulate "list var" command				
				packet = [0x00, self.address, 0x03, 0x06, 0x83, 0x03, 0x03, 0x03, 0x03, 0x83]


				packet.append(self.checksum(packet))


			if command == 0x04:
				#Simulate "list group var" command
				packet = [0x00, self.address, 0x05, 0x00]
				packet.append(self.checksum(packet))

			if command == 0x08:
				#Simulate "list curves" command
				packet = [0x00, self.address, 0x09, 0x00]
				packet.append(self.checksum(packet))


			if command == 0x10:
				#Simulate response of 18 bits
				packet = [0x00, self.address, 0x11, 0x03, randint(0,3), randint(0,255), randint(0,255)]
				packet.append(self.checksum(packet))

			if command == 0x20:
				#Simulate a sucesfull write
				packet = [0x00, self.address, 0xE0, 0x00]
				packet.append(self.checksum(packet))

			response = bytearray(packet)
			clientsock.send(response)
			print "Response send"
	

sock = PUC()