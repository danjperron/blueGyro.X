# blueGyro.X

Accéléromètre, gyroscope et magnétomètre dans un oeuf communicant avec bluetooth

Espérimentation pour vérifier la possibilité de lire la trajectoire de notre oeuf
avec des capteurs.

Le code principal utilise un puce Microchip pour lire le I2C des capteurs et de les envoyer
via bluetooth à un Raspberry Pi pour la captation des données.

Le répertoire Catapulte contient l'information du lanceur utilisant un R/C servo et un digit
pour un count down.

Un vidéo sur youtube explique le fonctionnement du déclencheur de la catapulte.

https://www.youtube.com/watch?v=4Xm-DA170TM

D'autre vidéo et code viendront 





-------


Accelerometer, gyroscope and magnetometer on bluetooth


V1.01 The HMC5883 magnetometer is added . If you don't want it, just rem #define HMC5883L_ENABLE in HMC5883L.h

catapultMag.jpg is the schematic.
