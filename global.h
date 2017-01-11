/* 
 * File:   global.h
 * Author: pehladik
 *
 * Created on 12 janvier 2012, 10:11
 */

#ifndef GLOBAL_H
#define	GLOBAL_H

#include "includes.h"

/* @descripteurs des tâches */
extern RT_TASK tServeur;
extern RT_TASK tconnect;
extern RT_TASK tmove;
extern RT_TASK tenvoyer;
extern RT_TASK tverifierbatterie;
extern RT_TASK tcompteur;

/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;
extern RT_MUTEX mutexCompteur;

/* @descripteurs des sempahore */
extern RT_SEM semConnecterRobot;
extern RT_SEM semVerifierBatterie;
extern RT_SEM semCompteur;
extern RT_SEM semDeplacer;

/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @variables partagées */
extern int etatCommMoniteur;
extern int etatCommRobot;
extern int compteur;
extern DServer *serveur;
extern DRobot *robot;
extern DMovement *move;
extern DBattery *batterie;

/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TSERVEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TVERIFIERBATTERIE;
extern int PRIORITY_TCOMPTEUR;

#endif	/* GLOBAL_H */

