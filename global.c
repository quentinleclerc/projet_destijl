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
RT_TASK tcompteur;
RT_TASK trechargerwd;
RT_TASK ttraiterimage;
RT_TASK tcalibrationarena;

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;
RT_MUTEX mutexCompteur;
RT_MUTEX mutexImage;
RT_MUTEX mutexArena;

RT_SEM semConnecterRobot;
RT_SEM semVerifierBatterie;
RT_SEM semCompteur;
RT_SEM semDeplacer;
RT_SEM semRechargerWD;
RT_SEM semTraiterImage;
RT_SEM semArena;

RT_QUEUE queueMsgGUI;

int etatCommMoniteur = 1;
int etatCommRobot = 1;
int compteur = 0;
int etatImage = 0;

DArena *arena;
DRobot *robot;
DMovement *move;
DServer *serveur;
DCamera *camera;
DArena *arena;
DPosition *position;

int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR           = 40;
int PRIORITY_TRECHARGERWD       = 36;
int PRIORITY_TENVOYER           = 35;
int PRIORITY_TCONNECT           = 30;
int PRIORITY_TCOMPTEUR          = 25;
int PRIORITY_TMOVE              = 20;
int PRIORITY_TTRAITERIMAGE      = 17;
int PRIORITY_TARENA				= 15;
int PRIORITY_TVERIFIERBATTERIE  = 12;
