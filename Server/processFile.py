import firebase_admin
from firebase_admin import credentials
from firebase_admin import firestore
import json

# https://stackoverflow.com/a/4913653/5561570
from math import radians, cos, sin, asin, sqrt

def haversine(lon1, lat1, lon2, lat2):
	"""
	Calculate the great circle distance between two points 
	on the earth (specified in decimal degrees)
	"""
	# convert decimal degrees to radians 
	lon1, lat1, lon2, lat2 = map(radians, [lon1, lat1, lon2, lat2])

	# haversine formula 
	dlon = lon2 - lon1 
	dlat = lat2 - lat1 
	a = sin(dlat/2)**2 + cos(lat1) * cos(lat2) * sin(dlon/2)**2
	c = 2 * asin(sqrt(a)) 
	r = 6371 # Radius of earth in kilometers. Use 3956 for miles
	return c * r


# Use a service account
cred = credentials.Certificate('./firebase-credential.json')
firebase_admin.initialize_app(cred)

db = firestore.client()
users_ref = db.collection(u'log')
docs = users_ref.get()

DATA = {}

for doc in docs:
	id = doc.id[1:]
	if(id not in DATA):
		DATA[id] = doc.to_dict().copy()
	else:
		DATA[id].update(doc.to_dict())

CELLULAR_DATA = {}
for id in DATA:
	if("LAT" in DATA[id]):
		try:
			latDegree = int(DATA[id]["LAT"][:2])
			latMinute = float(DATA[id]["LAT"][2:-1])
			lat = float(latDegree + latMinute/60)

			if(DATA[id]["LAT"][-1:] == "S"):
				DATA[id]["LAT"] = lat * -1
			else:
				DATA[id]["LAT"] = lat

			lngDegree = int(DATA[id]["LNG"][:3])
			lngMinute = float(DATA[id]["LNG"][3:-1])
			lng = float(lngDegree + lngMinute/60)

			if(DATA[id]["LNG"][-1:] == "W"):
				DATA[id]["LNG"] = lng * -1
			else:
				DATA[id]["LNG"] = lng

			key = "{}_{}_{}".format(DATA[id]["networkCode"], DATA[id]["TAC"], DATA[id]["ECI"])

			if(key not in CELLULAR_DATA):
				CELLULAR_DATA[key] = []

			CELLULAR_DATA[key].append(DATA[id])
		except Exception as e:
			del DATA[id]["LAT"]

CELLULAR_SUMMARY = {}

for key in CELLULAR_DATA:
	centerCSQ = 0
	centerLocation = {}
	LAT = []
	LNG = []

	CELLULAR_SUMMARY[key] = {}

	for i, point in enumerate(CELLULAR_DATA[key]):
		if(point["CSQ"] != 99 and point["CSQ"] > centerCSQ):
			centerLocation = point
			centerCSQ = point["CSQ"]
		LAT.append(point["LAT"])
		LNG.append(point["LNG"])

	CELLULAR_SUMMARY[key] = {
		"center": {
			"lat": centerLocation["LAT"],
			"lng": centerLocation["LNG"]
		},
		"sequence": centerLocation["sequence"],
		"radius": haversine(max(LNG), max(LAT), min(LNG), min(LAT)) * 1000
	}

print(json.dumps({
	"points": DATA,
	"circle": CELLULAR_SUMMARY
}))