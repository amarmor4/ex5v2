//
// Created by yarden95c on 08/12/16.
//

#ifndef EX3_MENU_H
#define EX3_MENU_H

#include <iostream>
#include <list>
#include <vector>
#include <thread>
#include<pthread.h>
#include "Status.h"
#include "Map.h"
#include "TaxiCenter.h"
#include "Color.h"
#include "CarType.h"
#include "LuxuryCab.h"
#include "StandardCab.h"
#include "Socket.h"
//#include "ClientHandlertest.h"

using namespace std;

class GameFlow {
private:
    Map *grid;
    TaxiCenter *taxiCenter;
    Socket *comm;
    int time;
    int portNo;
    Bfs* bfs;
    pthread_t threads[];
    pthread_t threadsTrip[];
    int driversNum;
    int tripsNum;

public:
    pthread_mutex_t connection_locker;
    pthread_mutex_t list_locker;

    GameFlow(Map *map, int portNo);

    void establishCommunication(string str);

    void startGame();

    void recieveDrivers();

    void insertARide();

    void insertAVehicle();

    void printDriverLocation();

    void moveTheClock();

    void sendClientNewLocation();

    void killTheClient(int i);

    Socket* getSock();

    bool isFinish10();

    bool isFinishBuildThread();

    void resetFinish10();

    ~GameFlow();

};


#endif //EX3_MENU_H