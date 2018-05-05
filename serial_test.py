import serial, time, msvcrt

throttle = 1000
aileron = 1500
elevator = 1500
rudder = 1500

tg=25
ag=50
eg=50
rg=50

comPort = 'COM4'

try:
	arduino = serial.Serial(comPort, 115200, timeout=.01)
	time.sleep(1)
	while True:
		if msvcrt.kbhit():
			key = ord(msvcrt.getch())
			if key == 27:
				break
			if key == 114:
				throttle = 1000
				aileron = 1500
				elevator = 1500
				rudder = 1500
			elif key == 119:
				throttle += tg
			elif key == 97:
				rudder -= rg		 
			elif key == 115:
				throttle-=tg
			elif key == 100:
				rudder += rg
			elif key == 224:
				key = ord(msvcrt.getch())
				if key == 80:
					elevator -= eg
				elif key == 72:
					elevator += eg
				elif key == 77:
					aileron += ag
				elif key == 75:
					aileron -= ag			   
			if throttle < 1000:
				throttle = 1000
			if throttle > 1900:
				throttle = 1900
			if aileron < 1000:
				aileron = 1000
			if aileron > 2000:
				aileron = 2000
			if elevator < 1000:
				elevator = 1000
			if elevator > 2000:
				elevator = 2000
			if rudder < 1000:
				rudder = 1000
			if rudder > 2000:
				rudder = 2000
			command = "%i,%i,%i,%i" % (throttle, aileron, elevator, rudder)	 
			print command
			arduino.write(command + "\n")

finally:
	arduino.close()