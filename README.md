# blueGyro.X

Accéléromètre, gyroscope et magnétomètre dans un oeuf communicant avec bluetooth

Espérimentation pour vérifier la possibilité de lire la trajectoire de notre oeuf
avec des capteurs.

Le code principal utilise une puce Microchip pour lire le I2C des capteurs et de les envoyer
via bluetooth à un Raspberry Pi pour la captation des données.

Le répertoire 'catapult' contient l'information du lanceur utilisant un R/C servo et un digit
pour un count down.

Un vidéo sur youtube explique le fonctionnement du déclencheur de la catapulte.

https://www.youtube.com/watch?v=4Xm-DA170TM

D'autres vidéos et codes viendront 


V1.01 Ajout du magnétomètre.
V1.02 Ajout du la puce mémoire. La boîte noire.

N.B. il est possible de ne pas utiliser le magnétomètre et la boîte noire. Il sagit de faire un undefine.<br>
Pour le magnétomètre  ->  ajouter #undef HMC5883L_ENABLE  juste après le #define dans HMC5883L.h<br>
Pour le AT24CM02      ->  ajouter #undef AT24CM02_ENABLE juste après le #define dans AT24CM02.h




-------


Accelerometer, gyroscope and magnetometer on bluetooth


V1.01 The HMC5883 magnetometer is added . If you don't want it, just rem #define HMC5883L_ENABLE in HMC5883L.h

catapultMag.jpg is the schematic.

-------
V1.02 Eerom memory I.C. add-on.  This allow to store data in case of bluetooth signal lost.

