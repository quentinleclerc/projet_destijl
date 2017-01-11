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
RT_TASK trechargerwd;
RT_TASK ttraiterimage;

RT_MUTEX mutexEtat;
RT_MUTEX mutexMove;
RT_MUTEX mutexImage;

RT_SEM semConnecterRobot;
RT_SEM semVerifierBatterie;
RT_SEM semRechargerWD;

RT_QUEUE queueMsgGUI;

int etatCommMoniteur = 1;
int etatCommRobot = 1;
int etatImage = -1;
DRobot *robot;
DMovement *move;
DServer *serveur;
DCamera *camera;

int MSG_QUEUE_SIZE = 10;

int PRIORITY_TSERVEUR = 30;
int PRIORITY_TCONNECT = 20;
int PRIORITY_TMOVE = 10;
int PRIORITY_TENVOYER = 25;
int PRIORITY_TVERIFIERBATTERIE = 18;
int PRIORITY_TRECHARGERWD = 40;
int PRIORITY_TTRAITERIMAGE = 17;
