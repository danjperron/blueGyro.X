#!/usr/bin/python3

import io
import fcntl
import time

IOCTL_I2C_SLAVE = 0x0703


class i2c:

    def __init__(self, device, bus):

        self.fh = io.open("/dev/i2c-"+str(bus), "br+", buffering=0)

        # set device address

        fcntl.ioctl(self.fh, IOCTL_I2C_SLAVE, device)

    def write(self, values):
        self.fh.write(bytearray(values))

    def read(self, NumberOfByte):
        valueBytes = self.fh.read(NumberOfByte)
        return list(valueBytes)

    def close(self):
        self.fh.close()


class mcp3428:

    def __init__(self, I2CAddress, bus):
        self.I2CAddress = I2CAddress
        self.dev = i2c(I2CAddress, bus)

        self.CFG_STARTCONV = 0b10000000
        self.CFG_CHANNEL1 = 0b00000000
        self.CFG_CHANNEL2 = 0b00100000
        self.CFG_CHANNEL3 = 0b01000000
        self.CFG_CHANNEL4 = 0b01100000
        self.CFG_BITS12 = 0b00000000
        self.CFG_BITS14 = 0b00000100
        self.CFG_BITS16 = 0b00001000
        self.CFG_ONESHOT = 0b00000000
        self.CFG_CONTINOUS = 0b00010000
        self.CFG_GAIN1 = 0b00000000
        self.CFG_GAIN2 = 0b00000001
        self.CFG_GAIN4 = 0b00000010
        self.CFG_GAIN8 = 0b00000011

        self.channel = -1
        self.gain = 1
        self.bits = 16

    def __del__(self):
        self.dev.close()

    def startConversion(self, channel=1, gain=1, bits=16):
        # Ajuste Nomnbre de bits
        if bits == 12:
            control = self.CFG_BITS12
        elif bits == 14:
            control = self.CFG_BITS14
        else:
            # 16 bits par défaux
            bits = 16
            control = self.CFG_BITS16
        self.bits = bits

        control = control | self.CFG_ONESHOT

        # Ajuste le gain
        if gain == 8:
            control = control | self.CFG_GAIN8
        elif gain == 4:
            control = control | self.CFG_GAIN4
        elif gain == 2:
            control = control | self.CFG_GAIN2
        else:
            gain = 1
            control = control | self.CFG_GAIN1
        self.gain = gain

        # Selection du canal d'entrée
        if channel == 4:
            control = control | self.CFG_CHANNEL4
        elif channel == 3:
            control = control | self.CFG_CHANNEL3
        elif channel == 2:
            control = control | self.CFG_CHANNEL2
        else:
            channel = 1
            control = control | self.CFG_CHANNEL1

        # avons-nous un canal différent
        # si oui changeons le canal et attendons
        # pour stabiliser le condensateur interne

        if channel != self.channel:

            # Appliquer le nouveau canal+ config sans conversion
            self.dev.write([control])

            # attendons un peut
            time.sleep(0.001)

        self.channel = channel
        # Ok pret pour une conversion

        control = control | self.CFG_STARTCONV
        self.dev.write([control])

    def waitForConversion(self):
        # Attendons pour la fin de la conversion
        while True:
            time.sleep(0.001)
            info = self.dev.read(3)
            if (info[2] & 0x80) == 0:
                break

    def readValue(self):
        # ok conversion fait
        # Lecture de la conversion
        info = self.dev.read(2)
        UnsignedV = info[0] * 256 + info[1]

        # de valeur non signé à valeur signé
        if(UnsignedV > 32767):
            UnsignedV = UnsignedV - 65536

        # Ajustement de la ponderation au nombre de bits
        bitsFactor=1
        if self.bits == 12:
            bitsFactor = 16
        elif self.bits == 14:
            bitsFactor = 4

        # retour d la valeur en voltage
        return 2.048 / 32768.0 * UnsignedV / self.gain * bitsFactor

    def startReadValue(self, channel=1, gain=1, bits=16):
        self.startConversion(channel, gain, bits)

        # Attente initiale  dépendant du nombe de bits
        if bits == 12:
            time.sleep(1.0/328.0)
        elif bits == 14:
            time.sleep(1.0/82.0)
        else:
            time.sleep(1.0/20.5)

        # Vérification de la fin de la conversion
        self.waitForConversion()

        # retourne la valeur
        return self.readValue()

if __name__ == "__main__":

    a2d = mcp3428(0x68,0)

    for i in range(4):
        print("Channel {} value is {:.5f} Volt".format(i+1,a2d.startReadValue(i+1)))

