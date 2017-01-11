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
extern RT_TASK trechargerwd;
extern RT_TASK ttraiterimage;
extern RT_TASK tcalibrationarena;

/* @descripteurs des mutex */
extern RT_MUTEX mutexEtat;
extern RT_MUTEX mutexMove;
extern RT_MUTEX mutexCompteur;
extern RT_MUTEX mutexImage;
extern RT_MUTEX mutexArena;

/* @descripteurs des sempahore */
extern RT_SEM semConnecterRobot;
extern RT_SEM semVerifierBatterie;
extern RT_SEM semCompteur;
extern RT_SEM semDeplacer;
extern RT_SEM semRechargerWD;
extern RT_SEM semTraiterImage;
extern RT_SEM semArena;
/* @descripteurs des files de messages */
extern RT_QUEUE queueMsgGUI;

/* @variables partagées */
extern int etatCommMoniteur;
extern int etatCommRobot;
extern int compteur;
extern int etatImage;

extern DServer *serveur;
extern DRobot *robot;
extern DMovement *move;
extern DCamera *camera;
extern DArena *arena;

/* @constantes */
extern int MSG_QUEUE_SIZE;
extern int PRIORITY_TSERVEUR;
extern int PRIORITY_TCONNECT;
extern int PRIORITY_TMOVE;
extern int PRIORITY_TENVOYER;
extern int PRIORITY_TVERIFIERBATTERIE;
extern int PRIORITY_TCOMPTEUR;
extern int PRIORITY_TRECHARGERWD;
extern int PRIORITY_TTRAITERIMAGE;
extern int PRIORITY_TARENA;
#endif	/* GLOBAL_H */

