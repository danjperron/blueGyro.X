#!/usr/bin/python3

import RPi.GPIO as GPIO

'''
    Application pour allumer les segments 
    d'un digit 7 segments avec les GPIOs.

    Copyright(c) 2016  Daniel Perron.

    Sous GPL license Version 3

    Le digit par défaux est le LTS-1720P
'''

class LED:

    def __init__(self, A=7, B=8, C=9, D=10, E=11, F=23, G=24, DP=25):
        self.A = A
        self.B = B
        self.C = C
        self.D = D
        self.E = E
        self.F = F
        self.G = G
        self.DP = DP
        GPIO.setwarnings(False)
        GPIO.setmode(GPIO.BCM)
        GPIO.setup(self.A, GPIO.OUT)
        GPIO.setup(self.B, GPIO.OUT)
        GPIO.setup(self.C, GPIO.OUT)
        GPIO.setup(self.D, GPIO.OUT)
        GPIO.setup(self.E, GPIO.OUT)
        GPIO.setup(self.F, GPIO.OUT)
        GPIO.setup(self.G, GPIO.OUT)
        GPIO.setup(self.DP, GPIO.OUT)

        # creation d'un dictionaire pour convertir le chiffre en segment
        #    segment
        #
        #
        #    -A-
        #   |   |
        #   F   B
        #   |   |
        #    -G-
        #   |   |
        #   E   C
        #   |   |
        #    -D-   DP
        #
        #
        #   creation de la correspondance digit versus segment
        #

        D0 = [A, B, C, D, E, F]
        D1 = [B, C]
        D2 = [A, B, G, E, D]
        D3 = [A, B, G, C, D]
        D4 = [F, B, G, C]
        D5 = [A, F, G, C, D]
        D6 = [A, F, G, C, D, E]
        D7 = [A, B, C]
        D8 = [A, B, C, D, E, F, G]
        D9 = [A, B, C, D, F, G]
        DMOINS = [G]
        DBLANK = []
        DQUESTION = [A, B, G, E]

        # dictionaire des valeurs
        self.Chiffre = {0: D0, 1: D1, 2: D2, 3: D3, 4: D4,
                        5: D5, 6: D6, 7: D7, 8: D8, 9: D9,
                        '-': DMOINS, ' ': DBLANK, '?': DQUESTION,
                        'SEGA': [A], 'SEGB': [B], 'SEGC': [C],
                        'SEGD': [D], 'SEGE': [E], 'SEGF': [F],
                        'SEGG': [G], 'SEGDP': [DP]}

        self.affiche(' ')

    def afficheSegments(self, segments=[]):
        # effacons les segments
        # utilisons le chiffre 8
        for segment in self.Chiffre[8]:
            GPIO.output(segment, True)
        # maintenant activons les bons segments
        if len(segments) > 0:
            for segment in segments:
                GPIO.output(segment, False)

    def affiche(self, Dval=' ', DP=False):
        if Dval in self.Chiffre:
            self.afficheSegments(self.Chiffre[Dval])
        else:
            self.afficheSegments(self.Chiffre['?'])  # n'affiche Question
        # et le DP
        GPIO.output(self.DP, not DP)


