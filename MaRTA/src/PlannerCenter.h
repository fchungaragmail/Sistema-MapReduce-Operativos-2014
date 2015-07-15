/*
 * PlannerCenter.h
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#ifndef PLANNERCENTER_H_
#define PLANNERCENTER_H_

#include "Utilities.h"

bool processMessage(Message *recvMessage);
void initPlannerCenter();
void _planificarHilo(void* args);

#endif /* PLANNERCENTER_H_ */
