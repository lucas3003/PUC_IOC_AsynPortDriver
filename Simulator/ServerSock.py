from socket import *
from array import *
import thread
import threading


#Software who simulate the basic commands from PUC (Placa Universal de Controle, in portuguese)
#Communication by ip and port specified on the last lines
class ServerSock:
	
	def start(self):
		while 1:
			self.sock.listen(2)				
			clientsock, addr = self.sock.accept()			
			thread.start_new_thread(self.HANDLER, (clientsock, addr))	
			
	def __init__(self, host, port, handler):		
		self.HOST = host
		self.PORT = port
		#self.BUFSIZ = 16391;
		self.ADDR = (self.HOST, self.PORT)
		self.sock = socket(AF_INET, SOCK_STREAM)
		self.sock.bind(self.ADDR)
		self.HANDLER = handler

		self.start()

			


