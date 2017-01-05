#include "fonctions.h"

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);

void envoyer(void * arg) {
    DMessage *msg;
    int err;

    while (1) {
        rt_printf("tenvoyer : Attente d'un message\n");
        if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
            rt_printf("tenvoyer : envoi d'un message au moniteur\n");
            serveur->send(serveur, msg);
            msg->free(msg);
        } else {
            rt_printf("Error msg queue write: %s\n", strerror(-err));
        }
    }
}

void connecter(void * arg) {
    int status;
    DMessage *message;

    rt_printf("tconnect : Debut de l'exécution de tconnect\n");

    while (1) {
        rt_printf("tconnect : Attente du sémarphore semConnecterRobot\n");
        rt_sem_p(&semConnecterRobot, TM_INFINITE);
        rt_printf("tconnect : Ouverture de la communication avec le robot\n");
        status = robot->open_device(robot);

        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        etatCommRobot = status;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {
            status = robot->start_insecurely(robot);
            if (status == STATUS_OK){
                rt_printf("tconnect : Robot démarrer\n");
                rt_printf("tconnect : Release sem verifier\n");
                rt_sem_v(&semVerifierBatterie);    
            }
        }

        message = d_new_message();
        message->put_state(message, status);

        rt_printf("tconnecter : Envoi message\n");
        message->print(message, 100);

        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
        }
    }
}

void communiquer(void *arg) {
    DMessage *msg = d_new_message();
    int receive_ok = 1;
    int num_msg = 0;
    int compteur = 0;

    rt_printf("tserver : Début de l'exécution de serveur\n");

     while (1) {
        
        //rt_printf("tconnect : Attente du sémarphore semServeurOpen\n");
        //rt_sem_p(&semServeurOpen, TM_INFINITE);
        //rt_printf("tconnect : Ouverture de la communication avec le robot\n");
        
        serveur->open(serveur, "8000");
        rt_printf("tserver : Connexion\n");

        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        etatCommMoniteur = STATUS_OK;
        rt_mutex_release(&mutexEtat);

        //REDONNER LES SEMAPHORES (batterie watchdog)


        while (receive_ok > 0) {
            rt_printf("tserver : Attente d'un message\n");
            receive_ok = serveur->receive(serveur, msg); //receive renvoie le nombre d'octets (0 ou inf si erreur)
            num_msg++;
                switch (msg->get_type(msg)) {
                    case MESSAGE_TYPE_ACTION:
                        rt_printf("tserver : Le message %d reçu est une action\n",
                                num_msg);
                        DAction *action = d_new_action();
                        action->from_message(action, msg);
                        switch (action->get_order(action)) {
                            case ACTION_CONNECT_ROBOT:
                                rt_printf("tserver : Action connecter robot\n");
                                rt_sem_v(&semConnecterRobot);
                                break;
                        }
                        break;
                    case MESSAGE_TYPE_MOVEMENT:
                        rt_printf("tserver : Le message reçu %d est un mouvement\n",
                                num_msg);
                        rt_mutex_acquire(&mutexMove, TM_INFINITE);
                        move->from_message(move, msg);
                        move->print(move);
                        rt_mutex_release(&mutexMove);
                        break;
                }
        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        etatCommMoniteur = STATUS_ERR_SELECT; 
        rt_mutex_release(&mutexEtat);

        // PRENDRE TOUS LES SEMAPHORES DE COMMUNICATION (batterie et watchdog)

        serveur->close(serveur) ;
        

        }
    }
}

void deplacer(void *arg) {
    int status = 1;
    int gauche;
    int droite;
    DMessage *message;

    rt_printf("tmove : Debut de l'éxecution de periodique à 1s\n");
    rt_task_set_periodic(NULL, TM_NOW, 1000000000);

    while (1) {
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("tmove : Activation périodique\n");

        rt_mutex_acquire(&mutexEtat, TM_INFINITE);
        status = etatCommRobot;
        rt_mutex_release(&mutexEtat);

        if (status == STATUS_OK) {
            rt_mutex_acquire(&mutexMove, TM_INFINITE);
            switch (move->get_direction(move)) {
                case DIRECTION_FORWARD:
                    gauche = MOTEUR_ARRIERE_LENT;
                    droite = MOTEUR_ARRIERE_LENT;
                    break;
                case DIRECTION_LEFT:
                    gauche = MOTEUR_ARRIERE_LENT;
                    droite = MOTEUR_AVANT_LENT;
                    break;
                case DIRECTION_RIGHT:
                    gauche = MOTEUR_AVANT_LENT;
                    droite = MOTEUR_ARRIERE_LENT;
                    break;
                case DIRECTION_STOP:
                    gauche = MOTEUR_STOP;
                    droite = MOTEUR_STOP;
                    break;
                case DIRECTION_STRAIGHT:
                    gauche = MOTEUR_AVANT_LENT;
                    droite = MOTEUR_AVANT_LENT;
                    break;
            }
            rt_mutex_release(&mutexMove);

            status = robot->set_motors(robot, gauche, droite);

            if (status != STATUS_OK) {
                if(nbErreurs < 3){
                    rt_mutex_acquire(&mutexNbErreurs, TM_INFINITE);
                    nbErreurs++;
                    rt_mutex_release(&mutexEtat);
                }

                else{
                    rt_mutex_acquire(&mutexEtat, TM_INFINITE);
                    etatCommRobot = status;
                    rt_mutex_release(&mutexEtat);

                    message = d_new_message();
                    message->put_state(message, status);

                    rt_printf("tmove : Envoi message\n");
                    if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                        message->free(message);
                    }
                }
            }
            else{
                rt_mutex_acquire(&mutexNbErreurs, TM_INFINITE);
                nbErreurs++;
                rt_mutex_release(&mutexEtat);
            }
        }
    }
}

// Batterie, periode 250ms, Low-priority
void verifierbatterie(void *arg) {
    int status = 0;
    int vbat;
    DMessage *message;

    rt_printf("tbatterie : Debut de l'éxecution de periodique à 250ms\n");
    rt_task_set_periodic(NULL, TM_NOW, 250000000);
       
    
    while(1){
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("tbatterie : Activation périodique\n");

        // attente semaphore
        rt_printf("tbatterie : Attente du sémarphore semVerifierBatterie\n");
        rt_sem_p(&semVerifierBatterie, TM_INFINITE);
        rt_printf("tbatterie : Get semaphore batterie\n");
        
        status = robot->get_vbat(robot,&vbat);
        
        // status presque toujours 0, mais vbat risque -1
        if ((status == STATUS_OK)&&(vbat != BATTERY_LEVEL_UNKNOWN)) {
            batterie->set_level(batterie,vbat);
            
            message = d_new_message();
            message->put_battery_level(message, batterie);
                
            rt_printf("tbatterie : Envoi message\n");
            if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                message->free(message);
            }
        }
        
        rt_sem_v(&semVerifierBatterie);
    }
}

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size) {
    void *msg;
    int err;

    msg = rt_queue_alloc(msgQueue, size);
    memcpy(msg, &data, size);

    if ((err = rt_queue_send(msgQueue, msg, sizeof (DMessage), Q_NORMAL)) < 0) {
        rt_printf("Error msg queue send: %s\n", strerror(-err));
    }
    rt_queue_free(&queueMsgGUI, msg);

    return err;
}
