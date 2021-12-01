from socket import *

serverName = '10.0.0.19'
# serverName = 'localhost'
serverPort = 5679
clientSocket = socket(AF_INET, SOCK_STREAM)
clientSocket.connect((serverName, serverPort))
# username = input('Input username: ')
# password = input('Input password: ')
jsonData = "{\"username\":\"Dan\", \"password\":\"1234\"}"
clientSocket.send(str.encode(jsonData))
comparison = clientSocket.recv(1024)
print('From Server: ', comparison)
clientSocket.close()
