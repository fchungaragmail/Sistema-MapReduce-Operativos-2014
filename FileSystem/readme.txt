Antes de ejecutar el FileSystem, debe configurarse a traves del FileSystem.config.
En el mismo se deben configurar los siguientes parametros:
PUERTO_LISTEN: Puerto de escucha del FS
IP_LISTEN: IP de escucha del FS
LISTA_NODOS: Cantidad minima de nodos para que el FS este operativo

Ej:
PUERTO_LISTEN=3000
IP_LISTEN=192.168.0.102
LISTA_NODOS=3


Luego de estar debidamente configurado, se debe ejecutar el FS y luego los NODOS.
Al conectarse, los nodos son aniadidos como disponibles en el FS.

Una vez que se llego a la cantidad minima de nodos, 
debe formatearse el FS con el comando "format"
Este comando formateara todos los espacios de datos de los nodos.

Para importar archivos al FS debe ejecutarse el comando "import" que tiene la siguiente forma:
import rutaDelArchivoLocal rutaDelArchivoEnMDFS

Ej:
import ./201301hourly.txt /201301hourly.txt

Una vez importado el archivo, se mostrara un cuadro de informacion.


INFO:
El estado del FS es siempre persistido en el archivo FileSystem.persist.
Lo que quiere decir que si el FS es cerrado y luego abiero, este va a conservar su ultimo estado,
a excepcion del estado de los nodos, que estaran no disponibles hasta que se reconecten.

Para volver al estado inical el FS, se debe borrar el archivo FileSystem.persist con el FS apagado.