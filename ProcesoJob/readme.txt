*Completar el archivo config.ini con la informacion necesaria.

*Comentar o descomentar los flags BUILD_CON_MOCK_MARTA o BUILD_CON_MOCK_NODO de Definiciones.h para buildear con mocks de estos procesos.
Lo que se mockea son los mensajes recibidos por parte de estos, el procesamiento de job es el mismo.

*El flag FALLO_EN_NODO_JOB_MUERTO, agrega la funcionalidad para terminar el job si algun nodo fallo, ya sea por conexion o porque informo que fallo.
El Job se cierra una vez que reporto todos los hilos que tenia ejecutando.
