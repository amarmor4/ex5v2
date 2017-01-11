//
// Created by yarden95c on 08/12/16.
//

#ifndef EX3_MENU_H
#define EX3_MENU_H

#include <iostream>
#include <list>
#include <vector>
#include <thread>
#include "Status.h"
#include "Map.h"
#include "TaxiCenter.h"
#include "Color.h"
#include "CarType.h"
#include "LuxuryCab.h"
#include "StandardCab.h"
#include "Socket.h"

using namespace std;

class GameFlow {
private:
    Map *grid;
    TaxiCenter *taxiCenter;
    Socket *comm;
    vector<Socket *> sockVector;
    int time;
    int portNo;
    Bfs* bfs;
    vector<unique_ptr<thread>> threads;

public:
    GameFlow(Map *map, int portNo);

    void establishCommunication(string str);

    void startGame();

    void recieveDrivers();

    void insertARide();

    void insertAVehicle();

    void printDriverLocation();

    void moveTheClock();

    void sendClientNewLocation();

    void killTheClient();

    Socket* getSock();

    ~GameFlow();

};


#endif //EX3_MENU_H
