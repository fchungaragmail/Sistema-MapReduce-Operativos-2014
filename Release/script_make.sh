#!/bin/bash
#Se instalan las commons
cd so-commons-library/
sudo make install
cd ..

#Debe estar creada la  carpeta Debug con los makeFile de eclipse
#Se compila nodo
cd tp-2015-1c-the-byteless/NODO/Debug/
make all
mv NODO ..
cd ../../..

#Se compila Fs
cd tp-2015-1c-the-byteless/FileSystem/Debug/
make all
mv FileSystem ..
cd ../../..

#Se compila Marta
cd tp-2015-1c-the-byteless/MaRTA/Debug/
make all
mv MaRTA ..
cd ../../..

#Se compila Job
cd tp-2015-1c-the-byteless/ProcesoJob/Debug/
make all
mv ProcesoJob ..
cd ../../..



