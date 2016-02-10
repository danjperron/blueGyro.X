#!/usr/bin/python3

''' 
    Application pour faire un count-down et
    activer une catapulte avec un RC/Servo

    Copyright(c) 2016  Daniel Perron.

    Sous GPL license Version 3


    Le script python LED.py contenant 
    la classe LED est necessaire
    
    Pour importer la library RPIO:
    sudo pip-3.2 install RPIO   
'''

import time
from LED import LED
import RPIO
import RPIO.PWM
import signal

# enlever les messages
RPIO.PWM.set_loglevel(1)

buttonGPIO = 22

# initialisation du systeme
digit = LED()
servo = RPIO.PWM.Servo()
RPIO.setup(buttonGPIO, RPIO.IN, pull_up_down=RPIO.PUD_UP)

Compteur = 0


# creons un signal handler pour capture ctrl-c
def signal_handler(signal, frame):
    digit.affiche()
    quit()


signal.signal(signal.SIGINT, signal_handler)

while RPIO.input(buttonGPIO):
    time.sleep(0.2)
    # Un light chaser indicateur pour afficher 'READY'
    Compteur = (Compteur+1) % 6
    digit.affiche('SEG'+chr(65+Compteur))

# ok nous partons le count down

for compte in range(9, 0, -1):
    digit.affiche(compte)
    time.sleep(1.0)

digit.affiche(0)
servo.set_servo(18, 1500)
time.sleep(3)
servo.set_servo(18, 1100)
time.sleep(1)

digit.affiche()
RPIO.cleanup()


