#!/usr/bin/python3
import datetime
import asyncio
import os
import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore

# Use a service account
cred = credentials.Certificate('./firebase-credential.json')
firebase_admin.initialize_app(cred)

db = firestore.client()

@asyncio.coroutine
def addDoc(id, doc):
	doc_ref = db.collection(u'log').document(id)
	doc_ref.set(doc)
	try:
		fbDoc = doc_ref.get()
		print(u'{} => {}'.format(fbDoc.id, fbDoc.to_dict()))
	except google.cloud.exceptions.NotFound:
		print(u'No such document!')


import socket

server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
server_socket.bind(('', os.getenv('PORT', 8005)))

loop = asyncio.get_event_loop()
startTime = datetime.datetime.now()

while True:
	message, address = server_socket.recvfrom(1024)

	data = message.decode("utf-8").split(',')
	try:
		if(data[0] in ['G', 'C'] ):
			difftime = datetime.datetime.now() - startTime
			if(data[0] == 'C'):
				doc = {
					"sequence":    int(data[1], 36),
					"networkCode": int(data[2], 36),
					"TAC":    hex(int(data[3], 36)),
					"ECI":    hex(int(data[4], 36)),
					"CSQ":         int(data[5], 36),
					"record_minute": int(difftime.seconds / 60)
				}
			else:
				doc = {
					"sequence": int(data[1], 36),
					"LAT": data[2],
					"LNG": data[3],
					"record_minute": int(difftime.seconds / 60)
				}
			print("Doc => {}".format(doc))
			loop.run_until_complete(addDoc(
				data[0]+str(int(str(data[1]), base=36))+'-'+startTime.timestamp(),
				doc
			))

			result = doc
		else:
			result = data
	except Exception as e:
		raise
		result = data
	
	# result = {
	# 	"sequence":    int(data[0], 36),
	# 	"networkCode": int(data[1], 36),
	# 	"TAC":    hex(int(data[2], 36)),
	# 	"ECI":    hex(int(data[3], 36)),
	# 	"CSQ":         int(data[4], 36)
	# }


	print("Receive %s -> %s : %s"%(address, message, result))
	# server_socket.sendto(message.upper(), address)
