/*
 * PlannerCenter.c
 *
 *  Created on: 23/5/2015
 *      Author: utnso
 */

#include <stdio.h>
#include <stdlib.h>

#include "Utilities.h"

void processMessage(Message *recvMessage)
{
	switch ((*recvMessage).head) {
		case K_NewConnection:
			printf("PlannerCenter : planificar NewConnection\n");
			printf("***************\n");
			break;
		case K_FSMessage:
			printf("PlannerCenter : planificar FSMessage\n");
			printf("***************\n");
			break;
		case K_JobMessage:
			printf("PlannerCenter : planificar JobMessage\n");
			printf("***************\n");
			break;
		default:
			printf("PlannerCenter: ERROR !! message no identificado !!\n");
			printf("***************\n");
			break;
	}

}
