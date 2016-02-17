import io
import fcntl
import sys



# bus i2c a 1 par defaux

i2cbus = 1

# Allons chercher les parametres de la console

if len(sys.argv) > 2:
    print("Usage:\n\ti2cdetect.py [i2cbus(0 ou 1)]")
    quit()

if len(sys.argv) == 2:
    i2cbus = int(sys.argv[1])

# Verifions si l'adresse du bus i2c est valide
if ( i2cbus < 0) or (i2cbus > 1):
    print("L'adresse du bus I2C est invalide! (0 ou 1)")
    quit()

# maintenant ouvrons le bus

try:
    fh = io.open("/dev/i2c-"+str(i2cbus),"br+",buffering=0)
except:
    print("Ne peut ouvrir /dev/i2c-"+str(i2cbus))
    quit()

# creons l'entete
print("    ",end='')
for i in range(16):
    print("{:2X} ".format(i),end='')

# la boucle de scan
for i in range(0x80):
    if (i % 0x10) == 0:
        # affichons l'adresse haute
        print("\n{:02X}: ".format(i),end='')
    # selection de l'adresse de la peripherique
    i2cadd = fcntl.ioctl(fh, 0x0703, i)

    # verifions si il y a une peripherique a cette adresse   
    try:
        valeur = fh.read(1)
        print("{:02X} ".format(i), end='')
    except:
        print("-- ",end='')

# fin de ligne et voila
print()
fh.close()

