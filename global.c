/*
 * File:   global.h
 * Author: pehladik
 *
 * Created on 21 avril 2011, 12:14
 */

#include "global.h"

RT_TASK tServeur;
RT_TASK tconnect;
RT_TASK tmove;
RT_TASK tenvoyer;
RT_TASK tverifierbatterie;

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;
RT_MUTEX mutexCompteur;

RT_SEM semConnecterRobot;
RT_SEM semVerifierBatterie;
RT_SEM semCompteur;

RT_QUEUE queueMsgGUI;

int etatCommMoniteur = 1;
int etatCommRobot = 1;
int compteur = 0;
DRobot *robot;
DMovement *move;
DServer *serveur;
DBattery *batterie;

int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 30;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;
int PRIORITY_TCOMPTEUR = 15;
int PRIORITY_TVERIFIERBATTERIE = 1;
