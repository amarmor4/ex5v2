//
// Created by yarden95c on 08/12/16.
//

#include "GameFlow.h"
#include "Udp.h"
#include "Tcp.h"

int option;
vector<bool> finish_10;
//vector<ClientHandler*> handlers;
using namespace std;

class ClientHandler{
public:
    Socket* sock;
    TaxiCenter* taxiCenter;
    GameFlow* flow;
    int index;
    ClientHandler(Socket* inputSock, TaxiCenter* inputCenter, GameFlow* game,
                  int i) {
        this->sock = inputSock;
        this->taxiCenter = inputCenter;
        this->flow = game;
        this->index=i;
    }
    ~ClientHandler()   {
    }
};

vector<ClientHandler*> handlers;

/**
 * constractur of game flow.
 * @param map map of the city
 * @return nothing
 */
GameFlow::GameFlow(Map *map, int portNo1) {
    this->bfs=new Bfs(map);
    this->taxiCenter = new TaxiCenter(this->bfs);
    this->comm = NULL;
    this->time = 0;
    this->driversNum=0;
    this->portNo = portNo1;
    pthread_mutex_init(&this->connection_locker, 0);
    pthread_mutex_init(&this->list_locker, 0);
}

/**
 * destractur
 */
GameFlow::~GameFlow() {
    option=NULL;
    pthread_mutex_destroy(&this->connection_locker);
    pthread_mutex_destroy(&this->list_locker);
    delete(bfs);
    delete (comm);
    delete (taxiCenter);

    while(!handlers.empty()||!finish_10.empty()){
        ClientHandler *c=handlers.back();
        handlers.pop_back();
        delete (c);
        finish_10.pop_back();
    }


}

void GameFlow::establishCommunication(string str) {
    if (str=="UDP"){
        comm = new Udp(true, portNo);
    }
    else if (str=="TCP"){
        comm = new Tcp(true, portNo);
    }

}

/**
 * menu optinos of program
 */
void GameFlow::startGame() {
    //establishCommunication();
    //comm->initialize();
    int choice;
    cin >> choice;
    option = choice;
    while (choice != 7) {
        switch (choice) {
            case 1:
                this->recieveDrivers();
                break;
            case 2:
                this->insertARide();
                break;
            case 3:
                this->insertAVehicle();
                break;
            case 4:
                this->printDriverLocation();
                break;
            case 9:
                this->moveTheClock();
                //this->sendClientNewLocation();
            default:
                break;
        }
        cin >> choice;
    }
    //do join and exit from all threads
    for(int i=0; i<this->driversNum;i++){
        this->killTheClient(i);
        //pthread_join(this->threads[i], NULL);
    }
}

void *test(void* ptr){
    char buffer[9999];
    ClientHandler* handler = (ClientHandler*)ptr;
    handler->sock->reciveData(buffer, sizeof
    (buffer), handler->index);
    Driver *d2;
    boost::iostreams::basic_array_source<char> device(buffer,
                                                      sizeof(buffer));
    boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(
            device);
    boost::archive::binary_iarchive ia(s);
    ia >> d2;
    int driverId=d2->getDriverId();
    pthread_mutex_lock(&handler->flow->list_locker);
    handler->taxiCenter->addDriverInfo(d2);
    pthread_mutex_unlock(&handler->flow->list_locker);
    //send the cab

    Cab *cab = handler->taxiCenter->getDriver(driverId)->getCab();
    string serial_str;
    boost::iostreams::back_insert_device<std::string> inserter(serial_str);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std
    ::string> > stream(inserter);
    boost::archive::binary_oarchive oa(stream);
    oa << cab;
    stream.flush();
    handler->sock->sendData(serial_str, handler->index);
    while (option != 7) {
        if (option == 10 && !finish_10[handler->index]) {
            Point *newLocation = handler->taxiCenter->getDriver(driverId)
                    ->getCurrentLocation();
            string serial_str;
            boost::iostreams::back_insert_device<std::string> inserter(serial_str);
            boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> >
                    s(inserter);
            boost::archive::binary_oarchive oa(s);
            oa << newLocation;
            s.flush();
            pthread_mutex_lock(&handler->flow->list_locker);
            handler->sock->sendData(serial_str, handler->index);
            finish_10[handler->index]=true;
            pthread_mutex_unlock(&handler->flow->list_locker);
        }
    }
    pthread_exit(ptr);
}


/**
 * get a new driver from the client, set him a car and send the cab to the
 * client.
 */
void GameFlow::recieveDrivers() {
    int numOfDrivers;
    cin >> numOfDrivers;
    char buffer[9999];
    this->establishCommunication("TCP");
    this->comm->initialize();
    for (int i=0; i<numOfDrivers;i++) {
        this->comm->acceptClient();
        ClientHandler* handler = new ClientHandler(this->comm,
                                                   this->taxiCenter, this, i);
        handlers.push_back(handler);
        pthread_create(&this->threads[i], NULL, test, (void*)handler);
        //pthread_join(this->threads[i], NULL);
        finish_10.push_back(true);
    }
    driversNum=numOfDrivers;
}

/**
 * insert a new trip to the trips in the taxi center
 * trip doesn't have a driver yet, only in choice 6 we add a driver to
 * each trip
 */
void GameFlow::insertARide() {
    int id, xStart, yStart, xEnd, yEnd, numOfPassengers, startTime;
    double tariff;
    char c;
    cin >> id >> c >> xStart >> c >> yStart >> c >> xEnd >> c >> yEnd >> c
        >> numOfPassengers >> c >> tariff >> c >> startTime;
    Point *start = new Point(xStart, yStart);
    Point *end = new Point(xEnd, yEnd);
    Trip *trip = new Trip(this->bfs, id, start, end, numOfPassengers, tariff,
                          startTime);
    taxiCenter->addTrip(trip);
    start = NULL;
    end = NULL;
    trip = NULL;
    // set the pointers to point on null so when
    // the objects will delete those pointers will no longer point on them
}

/**
 * insert a new vehicle to the cabs in the taxi center
 */
void GameFlow::insertAVehicle() {
    int id, taxiType;
    char manufacturerLetter, colorLetter, c;
    enum Color color;
    enum CarType carType;
    cin >> id >> c >> taxiType >> c >> manufacturerLetter >> c >> colorLetter;
    color = Color(colorLetter);
    carType = CarType(manufacturerLetter);
    Cab* cab = new Cab(id, carType, color, taxiType, 1);
    taxiCenter->addCab(cab);
    cab = NULL;
}

/**
 *printing the currrent location of a specific driver
 */
void GameFlow::printDriverLocation() {
    vector<Driver *> v = taxiCenter->getDriversInfo();
    int driverId;
    cin >> driverId;
    for (vector<Driver *>::iterator it = v.begin(); it != v.end(); it++) {
        if ((*it)->getDriverId() == driverId) {
            cout << *((*it)->getCurrentLocation()) << endl;
        }
    }
}

/**
 * move the clock + 1
 * check with all the trips, if it is their start time, connect them to a driver
 * if their time passed make them to move one step
 */
void GameFlow::moveTheClock() {
    while(!isFinish10()) { }
    // bfs threads are over - need to add
    vector<Trip *> trips = taxiCenter->getTrips();
    vector<Driver *> drivers = taxiCenter->getDriversInfo();
    for (int i = 0; i < trips.size(); i++) {
        if (trips[i]->getStartTime() == time) {
            trips[i]->setDriver(this->taxiCenter->getClosestDriver(
                    trips[i]->getStartPoint()));
        }
        if (trips[i]->getStartTime() <= time && !trips[i]->getPath()
                ->empty()) {
            trips[i]->moveOneStep();
        } else if (trips[i]->getPath()->empty()) {
            trips[i]->getDriver()->setAvailable(true);
            taxiCenter->popTrip();
        }
    }
    time++;
    resetFinish10();
    option = 10;
}


/**
 * after 7, we need to finish the program so send the client a null and it
 * will die too.
 */
void GameFlow::killTheClient(int i) {
    Point *newLocation = NULL;
    string serial_str;
    boost::iostreams::back_insert_device<std::string> inserter(serial_str);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(
            inserter);
    boost::archive::binary_oarchive oa(s);
    oa << newLocation;
    s.flush();
    comm->sendData(serial_str,i);
}

bool GameFlow::isFinish10(){
    bool all=true;
        for(int i=0; i<finish_10.size(); i++){
            if(finish_10[i]==false){
                all=false;
                break;
            }
        }
        return all;
    }

void GameFlow::resetFinish10(){
    for(int i=0; i<finish_10.size(); i++){
        finish_10[i]=false;
    }
}