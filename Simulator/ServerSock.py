from socket import *
from array import *
import thread
import threading
from random import *

#Software who simulate the basic commands from PUC (Placa Universal de Controle, in portuguese)
#Communication by ip and port specified on the last lines
class ServerSock:
	
	def handler(self, clientsock, addr):
		 while 1:			
			data = clientsock.recv(self.BUFSIZ)

			if not data:
				print "Nothing received"
				break
			
			print "Received: %s" %(data)
							
			for i in range(0,4):		
				print "Received on byte %d: %d " %(i, ord(data[i]))				
					
			print "End of data"	
			
			if self.operacao == "read":
				#Make a packet with random 18 bits
				payload1 = randint(0,3)
				payload2 = randint(0, 255)
				payload3 = randint(0,255)			
				checksum = (payload1+payload2+payload3+20+0x11+3)				
				checksum = checksum & 0xFF
				checksum = 0x100 - checksum
				packet = [0, 20, 0x11, 3, payload1, payload2, payload3, checksum&0xFF]
				
			else: 
				if self.operacao == "write" or self.operacao == "write_block_curve":
					packet = [0, 20, 0xE0, 0, 12]  #Simulate a successful write
			
				else: 
					if self.operacao == "read_block_curve":
						packet = [0, 20, 0x41, 0xFF, 1, 4] 
						for i in range(0,16384):
							packet.append(randint(0, 255))
						packet.append(0)						
				
					else:
						print "Invalid operation"
			
			print packet
			
			resposta = bytearray(packet)
			clientsock.send(resposta)
			print "Request send"
	
	def start(self):
		while 1:
			self.sock.listen(2)				
			clientsock, addr = self.sock.accept()			
			thread.start_new_thread(self.handler, (clientsock, addr))	
			
	def __init__(self, host, port):
		self.operacao = raw_input("Operation(read, write, read_block_curve or write_block_curve):");
		self.HOST = host
		self.PORT = port
		self.BUFSIZ = 16391;
		self.ADDR = (self.HOST, self.PORT)
		self.sock = socket(AF_INET, SOCK_STREAM)
		self.sock.bind(self.ADDR)		

sock = ServerSock('127.0.0.1', 6543)
sock.start()
