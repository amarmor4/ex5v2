//
// Created by moran on 14/01/17.
//

#include "ClientHandlertest.h"

ClientHandlertest::ClientHandlertest(Socket* inputSock, TaxiCenter* inputCenter,
                            GameFlow* game, int i) {
    this->flow = game;
    this->sock = inputSock;
    this->taxiCenter = inputCenter;
    this->index=i;
}
ClientHandlertest::~ClientHandlertest()   {
    delete(sock);
    delete(taxiCenter);
    delete(flow);
}

Socket* ClientHandlertest::getSock() {
    return this->sock;
}

GameFlow* ClientHandlertest::getFlow() {
    return this->flow;
}

TaxiCenter* ClientHandlertest::getTaxiCenter() {
    return this->taxiCenter;
}

int ClientHandlertest::getIndex() {
    return this->index;
}