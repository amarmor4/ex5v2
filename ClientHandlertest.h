//
// Created by moran on 14/01/17.
//

#ifndef EX5_CLIENTHANDLER_H
#define EX5_CLIENTHANDLER_H

#include <iostream>
#include "GameFlow.h"
#include "TaxiCenter.h"
#include "Socket.h"

using namespace std;
class ClientHandlertest {
private:
    Socket *sock;
    TaxiCenter *taxiCenter;
    GameFlow *flow;
    int index;
public:
    ClientHandlertest(Socket *inputSock, TaxiCenter *inputCenter, GameFlow *game,
                  int i);

    ~ClientHandlertest();

    Socket *getSock();

    TaxiCenter* getTaxiCenter();

    GameFlow* getFlow();

    int getIndex();
};



#endif //EX5_CLIENTHANDLER_H
