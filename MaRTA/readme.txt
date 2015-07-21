#MaRTA

1) Hay que configurar el archivo "confuracionMaRTA.config", hay que setear: 
	K_PUERTO_LOCAL : puerto por donde escucha MaRTA
	K_FS_IP : ip del FileSystem
	K_FS_PUERTO : puerto del FileSystem

2)	 Si quieren debuggear MaRTA, busquen el metodo "processMessage" en "PlannerCenter.c"
 	ahi van a encontrar un switch, en donde va a entrar dependiendo el tipo de respuesta que le llegue