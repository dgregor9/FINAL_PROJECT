##################
#ECE599 - Final Project
# Video Car
#David Gregory
##################
# Receiver Code - Run on Raspberry Pi#


import socket
import serial
import RPi.GPIO as GPIO
#import time
#from timeit import default_timer as timer

#UDP_IP = '127.0.0.1' #For Testing
UDP_IP = "10.10.10.44"  #Address of PI(address to listen for), commands come from User
UDP_PORT = 2362

opencm = '/dev/ttyACM0'

FWD_THRESHOLD = 0.1
RVS_THRESHOLD = -0.1

LFT_THRESHOLD = -0.1
RGT_THRESHOLD = 0.1

PWM0_PIN = 33  #Throttle
PWM1_PIN = 32  #Steering

STR_BIAS = 2 #multiplication factor
THR_BIAS = 2

StopDuty = 8
StraightDuty = 8.1

PAN_CENTER = 500
TILT_CENTER = 400

GPIO.setmode(GPIO.BOARD)  #Set pin.reference mode 
GPIO.setup(PWM0_PIN, GPIO.OUT) #Set pin mode
GPIO.setup(PWM1_PIN, GPIO.OUT) #set pin mode

PWM_THR = GPIO.PWM(PWM0_PIN, 60) # 60Hz duty cycle for neutral
PWM_STR = GPIO.PWM(PWM1_PIN, 60) # 60Hz duty cycle

PWM_THR.start(StopDuty)  #duty cycle to start
PWM_STR.start(StraightDuty)  #duty cycle to start


#Open up a socket connection w/ PC
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) # UDP

sock.bind((UDP_IP, UDP_PORT))

print'Waiting for Command'


# Listen for data on assigned port
while True:

    #RECEIVE command from remote control
	rxdata, address = sock.recvfrom(1024) # buffer size is 1024 bytes
	
	print(rxdata)
	
	#Seperate rxdata "String" into a "LIST"
	myrxlist = rxdata.split(" ")
	#print  myrxlist
	
	count = 0
	#Parse the received string
	for element in myrxlist:
		count += 1
		
		if element == 'button':		
		   if myrxlist[1] == "&":
			 PWM_THR.ChangeDutyCycle(StopDuty)
			 PWM_STR.ChangeDutyCycle(StraightDuty)
			 
		   elif myrxlist[1] == "@":		
			 PAN_TILT = [PAN_CENTER, TILT_CENTER]  #(X,Y) of touchpad
			 str1 = ' '.join(str(e) for e in PAN_TILT)
			 print str1
			 
			 ###  SEND PAN_TILT Command to Open_CM over USB ###
			 ser1 = serial.Serial(opencm, 9600, timeout=1)
			 if ser1.isOpen():
				ser1.write(str1)  #Send data packet
			 ser1.close()
			 
		   else:
		     pass
		 
		
 		elif element == 'joy':		  
		  ############Throttle##########
 		  #Check for Throttle command - left thumbstick
		  if myrxlist[1] == 'left':   #Left thumb stick used for throttle
			throttle_value = float(myrxlist[3])	  #Need Y coordinate,
			#Check for deadzone
			if throttle_value < RVS_THRESHOLD:		#Reverse
				NewDuty = StopDuty + (throttle_value*THR_BIAS) #NewDuty = StopDuty+(-value)
				PWM_THR.ChangeDutyCycle(NewDuty)
				print (NewDuty)
				
			elif throttle_value > FWD_THRESHOLD:	#Forward
				NewDuty = StopDuty + (throttle_value*THR_BIAS) #NewDuty = StopDuty+(-value)
				PWM_THR.ChangeDutyCycle(NewDuty)
				print (NewDuty)
				
			else:
				PWM_THR.ChangeDutyCycle(StopDuty)
							
		  #############STEERING############
		  #Check for Steer command - right thumbstick
		  elif myrxlist[1] == 'right':		#Right thumb stick
			steer_value = float(myrxlist[2])	#Need X coordiate
			if steer_value < LFT_THRESHOLD:
				NewDuty = StraightDuty - (steer_value*STR_BIAS) #NewDuty = neutral_value+(-value)
				PWM_STR.ChangeDutyCycle(NewDuty)
				print (NewDuty)
				
			elif steer_value > RGT_THRESHOLD:
				NewDuty = StraightDuty - (steer_value*STR_BIAS) #NewDuty = neutral + (value)
				PWM_STR.ChangeDutyCycle(NewDuty)
				print (NewDuty)
				
			else:
				PWM_STR.ChangeDutyCycle(StraightDuty)
		
		
		elif element == 'select':	
		  PWM_THR.ChangeDutyCycle(StopDuty)
		
		
		#Check for Pan/Tilt Commands
		elif element == 'touch':
		  PAN = PAN_CENTER - int((float(myrxlist[1])) * 500)	  	
		  TILT = TILT_CENTER - int((float(myrxlist[2])) * 200)
	
		  PAN_TILT = [PAN, TILT]  #(X,Y) of touchpad
		  #print PAN_TILT
		  str1 = ' '.join(str(e) for e in PAN_TILT)
		  print str1
		  
		  
		  ###  SEND PAN_TILT Command to Open_CM over USB ###
		  ser1 = serial.Serial(opencm, 9600, timeout=1)
		  if ser1.isOpen():
			ser1.write(str1)  #Send data packet
		  	#response = ser1.read(ser1.inWaiting())
		  	#print(response) 	#Echo should be returned from OpenCM
		  ser1.close()
		  
		  
		else:
		   pass	
		   
	

