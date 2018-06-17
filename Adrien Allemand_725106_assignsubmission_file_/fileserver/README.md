# PCO Labo 7 partie 2

###### Par Adrien Allemand et Loyse Krug

Pour cette deuxième partie du laboratoire, nous avons commencé par apporter quelques modifications a notre code de l'étape précédente :

##RequestDispatcherThread

Premièrement la classe `RequestDispatcherThread` a été modifiée pour maintenant utiliser un `ThreadPool` à qui elle donne les Requêtes "wrappées" dans une RunnableRequest qui implémente l'interface Runnable afin que ce dernier les fasse exécuter par ses `SlaveThread`.

```c++
void run() Q_DECL_OVERRIDE{
    while(true) {
        qDebug() << "Waiting for requests...";
        Request next = toTreatBuffer->get();
        ++i;
        qDebug() << "Request " << i << "recieved, giving it to ThreadPool";

        // donnes la requête au pool pourqu'il l'assigne à un thread, peut être bloquante
        pool.start(new RunnableRequest(next,treatedBuffer,i));
    }
}
```

## RunnableRequest

Comme mentionné ci-dessus nous utilisons la classe `RunnableRequest` afin d'encapsuler les Requêtes.

```c++
class RunnableRequest: public Runnable {
private:
    Request r;
    AbstractBuffer<Response>* a;
    int i;
public:
    RunnableRequest(Request r, AbstractBuffer<Response>* a, int i):r(r), a(a), i(i){ }
    ~RunnableRequest(){}

    QString id(){
        return QString::number(i);
    }

    void run(){
        RequestHandler handler(r, false);
        a->put(handler.handle());
    }
};
```

La rêquête encapsulée est exécutées puis la réponse est stockée dans le buffer de sortie à l'appel de la fonction `run()` de la `RunnableRequest`.

## ConcreteBuffer

parrallèlement le `ConcreteBuffer` s'est vu ajouté une méthode `bool tryPut(T item)` qui permet de refuser l'ajout d'objets de manière bloquante :

```c++
    bool tryPut(T item){
        // si le buffer est plein on retourne false
        if(free_space == 0){
            mutex->release();
            return false;		// pas la place
        }

        // [...] ajout au buffer
        
        return true;			// il y avait de la place
    }
```

Cette méthode est utilisée par le `FileServer` lors de la réception d'une nouvelle requête :

```c++
void FileServer::processTextMessage(QString message)
{
    QWebSocket *pClient = qobject_cast<QWebSocket *>(sender());
    if (hasDebugLog)
        qDebug() << "Message received:" << message;
    if (pClient) {
        Request req(message, pClient->origin());

        // passe par le tryPut avec possibilité de refus de la requête
        if(!requests->tryPut(req)){
            pClient->sendTextMessage(Response(req, "server overloaded,try later").toJson());
        }
    }
}
```

Cela va éviter que le serveur ne crash si il met en attente 10'000 requêtes : les clients verront un message qui les enjoint de réessayer plus tard. Mieux vaut en servir une partie que pas du tout !

## ThreadPool

Cette classe est au coeur de notre nouvelle implémentation du serveur de fichier.

Elle est principalement composé d'un **constructeur** qui initialise un tableau de pointeurs sur SlaveThreads à nullptr, d'un **destructeur** qui arrêtes proprement tout les threads qui sont entrain de run et d'une méthode `void start(Runnable* runnable)` qui permet de donner un `Runnable` à un des `SlaveThread` afin que ce dernier l'exécute.

Le ThreadPool se protège des accès concurents avec un moniteur de Hoare. Le principe est que le pool va parcourir chacun des `SlaveThread` de son tableau `slaves`. Si il rencontre un `nullptr` il va créer un nouveau `SlaveThread`. Ensuite il va tester si le `SlaveThread` est actuellement inoccupé à l'aide de la fonction `slaves[i]->isfree()` sur laquelle nous reviendrons dans la partie dédiée au `SlaveThread`. Si il est inocupé il va lui passer le `Runnable` a exécuter puis le réveiller avec la fonction `slaves[i]->wakeup()`. Si le pool parcours tout le tableau sans succès il va se mettre en pause car tout les `SlaveThreads` sont actuellement occupé et il va attendre qu'un d'entre eux lui signal qu'il a finit sa tâche et qu'il est libre avant de recommencer sa recherche.

```c++
void ThreadPool::start(Runnable *runnable) {
    poolMonitor->monitorIn();
    // fonction bloquante tant qu'on n'a pas trouvé de thread pour exécuter le Runnable
    bool threadfound = false;
    while(!threadfound){
        // parcours des slaveThreads pour en trouver un de libre
        for(int i = 0; i < maxThreads; i++){
            // si le thread[i] est null on le crée puis le démarre
            if(slaves[i] == nullptr){
                slaves[i] = new SlaveThread(poolMonitor, &c, i);
                slaves[i]->start();
            }
            // si le thread[i] est libre on lui donne la tache et lui signale de se réveiller
            if (slaves[i]->isfree()){				// test si le slave est libre
                slaves[i]->setRunnable(runnable);	 // si oui on lui set son runnable
                slaves[i]->wakeup();				// le réveil pour lui dire qu'il a du travail
                threadfound = true;					
                break;
            }
        }        
        // si on n'a toujours pas trouvé de thread libre on va placer le pool en attente
        if(!threadfound) {
            poolMonitor->wait(c);	// attente du signal d'un SlaveThread
        }
    }
    poolMonitor->monitorOut();
}
```

## SlaveThread

La classe `SlaveThread` est spécialement construite pour être utilisée par la classe `ThreadPool` elle hérite de la classe `QThread` et  est protégée par un moniteur de Mesa basé sur une `QWaitCondition slaveWait` et un `QMutex mutex`.

Outre son **constructeur** et son **destructeur**, elle fournis les méthodes suivantes :

```c++
bool isfree(){
    mutex->lock();
    bool temp = _isfree;	 // sauve l'état
    if(_isfree){			// si le thread était libre
        _isfree = !_isfree;		 // on le bloque pour la suite
    }
    mutex->unlock();
    return temp;		    // on retourne l'état
}
```

`bool isfree()` est appelée par le `ThreadPool` lorsque ce dernier désire tester si un `SlaveThread`est disponible pour lui affecter une tâche. Cette méthode retourne l'état du thread (**true** pour libre et **false** pour occupé) mais pas seulement.

Comme on est obligé de libérer le mutex avant de retourner de cette méthode on "réserve" le thread si celui-ci est effectivement libre au moment du test.

En effet, le mutex est relaché entre le moment ou le `ThreadPool` va tester cette méthode et le moment où il va réellement démarer le thread ainsi on veut garantire que le thread ne soit plus considéré comme libre pour la suite.

```c++

/**
* @brief setTimeToDie dit à un thread de s'arrêter proprement
*/
void setTimeToDie(){
    mutex->lock();
    isTimeToDie = true;
    mutex->unlock();
    slaveWait->wakeOne();
}
```

ce setter permet de modifier la variable `bool isTimeToDie` définie à **false** par défault. En plus elle signale le thread concerné qu'il doit se réveiller pour constater qu'il est l'heure de mourir. Cette méthode est utilisée par `ThreadPool` dans son destructeur pour terminer proprment les threads.

```c++
/**
* @brief wakeup lance un signal de réveil sur le slave
*/
void wakeup(){
    slaveWait->wakeOne();
}
```

`void wakeup()` est un simple signal  de réveil utilisable depuis l'extérieur sur la `QWaitCondition` du slavethread. Elle est appelée par le `ThreadPool` pour signaler au `SlaveThread` concerné qu'il a du travail.

```c++
void setRunnable(Runnable* r){
    mutex->lock();
    this->r = r;
    mutex->unlock();
}
```

`void setRunnable(Runnable* r)` est un simple setter qui change le `Runnable` du `SlaveThread` de maniètre protégée.

```c++
void run(){
    mutex->lock();
    while (!isTimeToDie){
        // tant que le slave n'est pas occupé il va se mettre en attente
        while(_isfree && !isTimeToDie){
            slaveWait->wait(mutex);     // attente que on lui signale qu'il a du travail
        }

        if(isTimeToDie){
            break;  // interruption si réveillé pour mourir
        }

        mutex->unlock();    // super important que le mutex soit unlock pour le r->run()
        r->run();           // exécution de la tâche

        mutex->lock();
        _isfree = true;     // passe son état a libre
        mutex->unlock();    // pour que le pool puisse faire le isfree() sans être bloqué

        poolMonitor->monitorIn();
        poolMonitor->signal(*poolWait);  // signal au pool que on est libre
        poolMonitor->monitorOut();

        mutex->lock();
    }
    mutex->unlock();
}
```

Finalement la fonction `run()`est le coeur du `SlaveThread`. Elle override la fonction de `QThread` et permet à un `SlaveThread` qui a été démarré de se mettre en attente tant qu'il n'a pas reçu de tâche, le cas échant de la traiter.

 En la parcourant depuis le début la logique est la suivante : Lorsqu'un `SlaveThread` est démarré, il est libre et va se placer en attente :

```c++
slaveWait->wait(mutex);     // attente que on lui signale qu'il a du travail
```

Il se fera réveillé par le `ThreadPool` lorsque celui-ci lui aura préalablement définit un nouveau runnable.

Une fois réveillé, il va exécuter la tâche qui lui a été confié 

```c++
mutex->unlock();    // super important que le mutex soit unlock pour le r->run()
r->run();           // exécution de la tâche
```

*Notons ici qu'il est impératif de relacher le mutex avant de faire le `r->run()` faute de quoi le `ThreadPool` sera bloqué si il apelle la fonction `isfree()` ce qui provoquerait une exécution séquentielle du programme.*

Finalement le `SlaveThread` va signaler au `ThreadPool` qu'il est libre afin que ce dernier puisse reprendre sa quête d'un `SlaveThread` libre si s'était mis en pause après n'en avoir point trouvé.

## Réponse aux questions

A) *Quelle  est  la  taille  optimale  du  thread  pool  pour  cette  application ?* 

Le nombre de thread optimal va dépendre du CPU. Le mien est un i7 hyperthreadé donc théoriquement capable de faire tourner 8 threads en parrallèles. En sachant que les slaveThreads ne sont pas les seuls a tourner dans le programme et qu'il y en a en plus une multitude d'autre qui tournent pour chaque programme de l'ordidateur, il est difficile de trouver un chiffre cohérent. Une petite valeur tel que 8 ou 10 permettra de minimiser les préhemptions autant que possible tout en occupant quand même le CPU de manière importante.

B) *Quels  facteurs  influencent ce choix ?*

Principalement le CPU, si on a la chance de posséder un i9 , un Xéon ou même un AMD on va pouvoir se lâcher, avec un i5 Quad-Core sans Hyperthreading ça va pas être top.

C) *Que se passe-t-il maintenant lorsque que l’on inonde le serveur de requêtes ?*

Il crash ... mais il en refuse certaines en demandant d'attendre un moment et de recommencer

D) *Constatez-vous une amélioration au niveau de la stabilité du serveur ?*

Malheureusement non, nous avons eu des problèmes depuis le premier laboratoire, même votre version initiale sensée fonctionner ne cessait de perdre la conexion après quelque requêtes simples. Nous pensons cependant que cela n'est pas du a notre code.

E) *et qu’en est-il au niveau del’occupation mémoire, par rapport à la version d’un thread par requête ?*

Diviser le nombre de threads actifs par 100 va réduire la mémoire utilisée. le fait de refuser les requêtes en attentes après que notre buffer de 127 soit plein réduit également cette valeur.