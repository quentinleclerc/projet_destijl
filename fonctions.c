#include "fonctions.h"

int write_in_queue(RT_QUEUE *msgQueue, void * data, int size);
void gestionCompteur(int status);


void envoyer(void * arg) {
    DMessage *msg;
    int err;

    while (1) {
        rt_printf("tenvoyer : Attente d'un message\n");
        if ((err = rt_queue_read(&queueMsgGUI, &msg, sizeof (DMessage), TM_INFINITE)) >= 0) {
            rt_printf("tenvoyer : envoi d'un message au moniteur\n");
            /* Fermeture du serveur si la connexion a été perdue */
            serveur->send(serveur, msg);
            msg->free(msg);
        } else {
            rt_printf("Error msg queue write: %s\n", strerror(-err));
        }
    }
}


/************************************************************ 
 * CONNECTER                                                *
 * Cette fonction s'occupe de la connexion au robot         *
 * Quand la connexion est établie elle permet aux threads   *
 * de commencer leur exécution.                             *
 * Ce thread est normalement executé une seule fois.        *                        
 ************************************************************/
void connecter(void * arg) {
    int status;
    DMessage *message;

    rt_printf("tconnect : Debut de l'exécution de tconnect\n");

    while (1) {
        rt_printf("tconnect : Attente du sémaphore semConnecterRobot\n");
        rt_sem_p(&semConnecterRobot, TM_INFINITE);
        rt_printf("tconnect : Ouverture de la communication avec le robot\n");

        status = robot->open_device(robot);
        gestionCompteur(status);

        if (status == STATUS_OK) {
            status = robot->start_insecurely(robot);
            //status = robot->start(robot);
            if (status == STATUS_OK){
                rt_printf("tconnect : Robot démarré\n");
                rt_printf("tconnect : Release sem verifier\n");

                rt_sem_v(&semRechargerWD);
                rt_sem_v(&semDeplacer);
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

/************************************************************ 
 * COMMUNIQUER                                              *
 * Cette fonction gère la communication entre le moniteur   *
 * et le superviseur.                                       *
 * Execution aperiodique de ce thread.                      *
 ************************************************************/
void communiquer(void *arg) {
    DMessage *msg = d_new_message();
    int size = 1;
    int num_msg = 0;
    int compteFindArena = 0;

    rt_printf("tserver : Début de l'exécution de serveur\n");
    serveur->open(serveur, "8000");
    rt_printf("tserver : Connexion\n");
    rt_sem_v(&semTraiterImage);

    while (size > 0) {
        rt_printf("tserver : Attente d'un message\n");
        size = serveur->receive(serveur, msg); //receive renvoie le nombre d'octets (0 ou inf si erreur)
        num_msg++;
        if(size >0) {
            switch (msg->get_type(msg)) {
                case MESSAGE_TYPE_ACTION:
                    rt_printf("tserver : Le message %d reçu est une action\n",num_msg);
                    DAction *action = d_new_action();
                    action->from_message(action, msg);
                    switch (action->get_order(action)) {
                        case ACTION_CONNECT_ROBOT:
                            rt_printf("tserver : Action connecter robot\n");
                            rt_sem_v(&semConnecterRobot);
                            break;
                        case ACTION_FIND_ARENA:
                            compteFindArena++;
                            rt_printf("tserver : Action trouver Arène\n");
                            if(compteFindArena <= 1)
                                rt_sem_p(&semTraiterImage, TM_INFINITE);
                            rt_sem_v(&semArena);
                            break;
                        case ACTION_ARENA_FAILED:
                            compteFindArena = 0;
                            rt_printf("tserver : Action arene failed\n");

                            rt_printf("tserver : semTraiterImage libéré \n");
                            rt_sem_v(&semTraiterImage);
                            break;
                        case ACTION_ARENA_IS_FOUND:
                            compteFindArena = 0;
                            rt_sem_v(&semTraiterImage);
                            break;
                        case ACTION_COMPUTE_CONTINUOUSLY_POSITION:
                            rt_mutex_acquire(&mutexImage, TM_INFINITE);
                            etatImage = ACTION_COMPUTE_CONTINUOUSLY_POSITION;
                            rt_printf("tserver : mutexImage obtenu \n");
                            rt_mutex_release(&mutexImage);
                            break;
                        case ACTION_STOP_COMPUTE_POSITION:
                            rt_mutex_acquire(&mutexImage, TM_INFINITE);
                            etatImage = ACTION_STOP_COMPUTE_POSITION;
                            rt_mutex_release(&mutexImage);
                            break;
                    } // _fin SWITCH Action
                case MESSAGE_TYPE_MOVEMENT:
                    rt_printf("tserver : Le message reçu %d est un mouvement\n",num_msg);
                    rt_mutex_acquire(&mutexMove, TM_INFINITE);
                    move->from_message(move, msg);
                    move->print(move);
                    rt_mutex_release(&mutexMove);
                    break;
                case MESSAGE_TYPE_MISSION:
                    break;
            } // _fin SWITCH type message
        } // _fin de boucle "IF"
    } // _fin de boucle "WHILE"
    serveur->close(serveur) ;
} // _fin de boucle "COMMUNIQUER"




/************************************************************* 
 * DEPLACER                                                  *
 * Cette fonction gère la communication entre le superviseur *
 * et le robot. Elle répond aux ordres de déplacement.       *
 * L'execution de ce thread est périodique 200ms.            *                    
 *************************************************************/
void deplacer(void *arg) {
    int status = 1;
    int gauche;
    int droite;

    rt_printf("tmove : Debut de l'éxecution de periodique à 1s\n");
    rt_task_set_periodic(NULL, TM_NOW, 2000000000);

    while (1) {
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("tmove : Activation périodique\n");

        rt_printf("tmove : Attente du sémaphore semDeplacer\n");
        rt_sem_p(&semDeplacer, TM_INFINITE);
        rt_printf("tmove : Sémaphore semDeplacer obtenu\n");

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
        gestionCompteur(status);

        rt_sem_v(&semDeplacer);
    }
}

/************************************************************* 
 * VERIFIER BATTERIE                                         *
 * Cette fonction indique l'état de batterie du robot au     *
 * superviseur.                                              *
 * L'execution de ce thread est périodique 250ms.            *                    
 *************************************************************/
void verifierbatterie(void *arg) {
    int status = 0;
    int vbat;
    DMessage *message;
    DBattery *batterie;

    rt_printf("tbatterie : Debut de l'éxecution de periodique à 250ms\n");
    rt_task_set_periodic(NULL, TM_NOW, 250000000);
    
    while(1){
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("tbatterie : Activation périodique\n");

        // attente semaphore
        rt_printf("tbatterie : Attente du sémarphore semVerifierBatterie\n");
        rt_sem_p(&semVerifierBatterie, TM_INFINITE);
        rt_printf("tbatterie : Semaphore batterie obtenu\n");
        
        status = robot->get_vbat(robot,&vbat);
        gestionCompteur(status);

        // status presque toujours 0, mais vbat risque -1
        if ( (status == STATUS_OK) && (vbat != BATTERY_LEVEL_UNKNOWN) ) {
            batterie = d_new_battery();
            batterie->set_level(batterie,vbat);
            
            message = d_new_message();
            message->put_battery_level(message, batterie);
                
            rt_printf("tbatterie : Envoi message\n");
            if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                message->free(message);
            }
            
            batterie->free(batterie);
        }
        
        rt_sem_v(&semVerifierBatterie);
    }
}

/************************************************************* 
 * COMPTEUR                                                  *
 * Cette fonction gère la deconnexion du robot, en empêchant *
 * l'exécution des threads côté superviseur si elle est      *
 * detectée perdue                                           *
 * L'execution de ce thread est apériodique, elle est        *
 * déclenchée par la fonction gestionCompteur(status)        *                    
 *************************************************************/
void threadCompteur(void * arg){
    DMessage *message;
    int status = STATUS_ERR_TIMEOUT;

    while(1){

        rt_printf("tcompteur : Attente du sémaphore semCompteur\n");
        rt_sem_p(&semCompteur, TM_INFINITE);
        rt_printf("tcompteur : Sémaphore semCompteur obtenu\n");

        message = d_new_message();
        message->put_state(message, status);

        rt_printf("tcompteur : Envoi message\n");
        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
        }

        rt_sem_p(&semVerifierBatterie, TM_INFINITE);
        rt_printf("tcompteur : sémaphore semVerifierBatterie obtenu\n");
        rt_sem_p(&semDeplacer, TM_INFINITE);
        rt_printf("tcompteur : sémaphore semDeplacer obtenu, robot stoppé\n");
        rt_sem_p(&semRechargerWD, TM_INFINITE);
        rt_printf("tcompteur : sémaphore semRechargerWD obtenu\n");
        rt_sem_p(&semArena, TM_INFINITE);
        rt_printf("tcompteur : sémaphore semArena obtenu\n");
        rt_sem_p(&semTraiterImage, TM_INFINITE);
        rt_printf("tcompteur : sémaphore semTraiterImage obtenu\n");
    }
}



/************************************************************* 
 * GESTION COMPTEUR                                          *
 * Fonction qui déclenche le thread compteur si la           *
 * deconnexion a été repérée, après 7 messages non transmis  *
 *************************************************************/
void gestionCompteur(int status){
    rt_printf("tcompteur : appel, status = %d\n", status);
    rt_mutex_acquire(&mutexCompteur, TM_INFINITE);
    if(status == STATUS_OK){
        compteur = 0;
    }
    else{
        if(compteur < 6){
            compteur ++;
            rt_printf("gestionCompteur : compteur++, compteur = %d\n", compteur);
        }
        else{
            rt_printf("tcompteur : relache semCompteur\n");
            rt_sem_v(&semCompteur);
        }
    }
    rt_mutex_release(&mutexCompteur);
    
}

/************************************************************* 
 * RECHARGER WatchDog                                        *
 * Fonction qui sert à recharger le watchdog du robot afin   *
 * de détecter une éventuelle déconnexion                    *
 * Exécution périodique du thread, toutes les 1s             *
 *************************************************************/
void rechargerwd(void *arg) {
    int status = 0;

    rt_printf("trechargerwd : Debut de l'éxecution de periodique à 1s\n");
    rt_task_set_periodic(NULL, TM_NOW, 1000000000);

    while(1){
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("trechargerwd : Activation périodique\n");

        // attente semaphore
        rt_printf("trechargerwd : Attente du sémarphore semRechargerWD\n");
        rt_sem_p(&semRechargerWD, TM_INFINITE);
        rt_printf("trechargerwd : Get semaphore semRechargerWD\n");

        status = robot->reload_wdt(robot);
        gestionCompteur(status);

        rt_sem_v(&semRechargerWD);
    }
}

/************************************************************* 
 * Traiter Image                                             *
 * Ce thread gère la récupération des images via la camera   *
 * Elles sont ensuite envoyées au moniteur                   *
 * L'exécution de ce thread est periodique 600 ms            *
 *************************************************************/
void traiterimage(void *arg) {
    int status = 0;
    DImage *image;
    DJpegimage *jpegimage;
    DMessage *message;
    DPosition *position;
    
    camera->open(camera);

    rt_printf("ttraiterimage : Debut de l'éxecution de periodique à 600ms\n");
    rt_task_set_periodic(NULL, TM_NOW, 600000000);

    while(1){
        /* Attente de l'activation périodique */
        rt_task_wait_period(NULL);
        rt_printf("ttraiterimage : Activation périodique\n");
        
        rt_printf("ttraiterimage : Attente du sémaphore semTraiterImage\n");
        rt_sem_p(&semTraiterImage, TM_INFINITE);
        rt_printf("ttraiterimage : Sémaphore semTraiterImage obtenu\n");
        
        image = d_new_image();
        jpegimage = d_new_jpegimage();
        
        camera->get_frame(camera,image);
           
        /* pour savoir s'il s'agit du calcul position */
        rt_mutex_acquire(&mutexImage, TM_INFINITE);
        status = etatImage;
        rt_mutex_release(&mutexImage);
        
        if (status == ACTION_COMPUTE_CONTINUOUSLY_POSITION){
       
            rt_printf("ttraiterimage : position\n");

            position = d_new_position();
            position = image->compute_robot_position(image,NULL);
            
            if (position != NULL){
                d_imageshop_draw_position(image,position);
                
                message = d_new_message();
                message->put_position(message, position);

                rt_printf("ttraiterimage : Envoi message position\n");
                if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
                    message->free(message);
                }
            }   
        }
        
        jpegimage->compress(jpegimage,image);

        message = d_new_message();
        message->put_jpeg_image(message, jpegimage);

        rt_printf("ttraiterimage : Envoi message image\n");
        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
        }
        
        image->free(image);
        jpegimage->free(jpegimage);
        rt_sem_v(&semTraiterImage);
    }
}


/************************************************************* 
 * Calibraion arène                                          *
 * La calibration de l'arène detècte l'arène avec les images *
 * en provenance de la caméra, et la trace sur l'écran du    *
 * moniteur.                                                 *
 * L'exécitop, de ce thread est apériodique, le semaphore    *
 * est liberé par le thread communiquer si le bouton         *
 * Chercher Arène est appuyé sur le moniteur                 *
 *************************************************************/
void calibrationArene(void *arg) {
    DImage *image;
    DMessage *message;
    DJpegimage *jpegim;
    DArena *arena;

    while(1){

        // attente semaphore
        rt_printf("tcalibrationarena : Attente du sémarphore semChercherArene\n");
        rt_sem_p(&semArena, TM_INFINITE);
        rt_printf("tcalibrationarena : Get semaphore arene\n");


        image = d_new_image();
        jpegim = d_new_jpegimage();
        camera->get_frame(camera,image);

        arena = d_new_arena();
        arena = image->compute_arena_position(image);
        d_imageshop_draw_arena(image,arena);

        jpegim->compress(jpegim,image);

        image->free(image);

        message = d_new_message();
        message->put_jpeg_image(message, jpegim);

        rt_printf("tcalibrationarena : Envoi message\n");
        if (write_in_queue(&queueMsgGUI, message, sizeof (DMessage)) < 0) {
            message->free(message);
        }
    }
}


/************************************************************* 
 * Write_in_queue                                            *
 * Fonction qui envoie des messages au moniteur              *
 * paramètres : queue de message, donnée, taille             *
 *************************************************************/
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