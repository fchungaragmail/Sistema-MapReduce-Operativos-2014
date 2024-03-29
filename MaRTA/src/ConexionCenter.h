/*
 * ConexionCenter.h
 *
 *  Created on: 22/5/2015
 *      Author: utnso
 */

#ifndef CONEXIONCENTER_H_
#define CONEXIONCENTER_H_

#include <commons/collections/list.h>
#include "Utilities.h"

void initConexiones();
int connectToFS();
void closeServidores();
Message* listenConnections();

#endif /* CONEXIONCENTER_H_ */
