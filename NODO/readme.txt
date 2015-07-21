Teniendo el archivo de NODO.config, correr nodo, se inicializa y se conecta a FS y lanza el hilo para FS, y luego se inicializa como servidor, quedando a la espera de solicitudes.

Crear archivo data.bin, para crearlo, posicionarse en el directorio donde se quiere crear y en la consola poner:

	truncate "nombre.bin" -s tamañoArchivo

tamañoArchivo -> 1GB, asi textual, si se quiere que sea de 1 gb

Ej: truncate "data.bin" -s 1GB
