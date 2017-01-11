//
// Created by yarden95c on 08/12/16.
//

int option;

#include "GameFlow.h"
#include "Udp.h"
#include "Tcp.h"

using namespace std;

/**
 * constractur of game flow.
 * @param map map of the city
 * @return nothing
 */
GameFlow::GameFlow(Map *map, int portNo1) {
    bfs=new Bfs(map);
    taxiCenter = new TaxiCenter(bfs);
    comm = NULL;
    time = 0;
    portNo = portNo1;
    sockVector=vector<Socket*>();
    //threads = vector<unique_ptr<thread>>();
}

/**
 * destractur
 */
GameFlow::~GameFlow() {
    delete (taxiCenter);
    delete (comm);
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
    this->killTheClient();

}

class ClientHandler{
public:
    Socket* sock;
    TaxiCenter* taxiCenter;
    ClientHandler(Socket* inputSock, TaxiCenter* inputCenter) {
        this->sock = inputSock;
        this->taxiCenter = inputCenter;
    }

};

void *test(void* ptr){
    char buffer[9999];
    ClientHandler* handler = (ClientHandler*)ptr;
    //this->sockVector.push_back(s);

    handler->sock->reciveData(buffer, sizeof(buffer));
    Driver *d2;
    boost::iostreams::basic_array_source<char> device(buffer,
                                                      sizeof(buffer));
    boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(
            device);
    boost::archive::binary_iarchive ia(s);
    ia >> d2;
    handler->taxiCenter->addDriverInfo(d2);
    //send the cab
    Cab *cab = handler->taxiCenter->getDriver()->getCab();
    string serial_str;
    boost::iostreams::back_insert_device<std::string> inserter(serial_str);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std
    ::string> > stream(inserter);
    boost::archive::binary_oarchive oa(stream);
    oa << cab;
    stream.flush();
    handler->sock->sendData(serial_str);
    cout << *(d2->getCurrentLocation()) << endl;

    while (option != 7) {
        if (option == 10) { //a flag that we done moving the clock  and
            // moving one step
            Point *newLocation = handler->taxiCenter->getDriver()
                    ->getCurrentLocation();
            string serial_str;
            boost::iostreams::back_insert_device<std::string> inserter(serial_str);
            boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> >
                    s(inserter);
            boost::archive::binary_oarchive oa(s);
            oa << newLocation;
            s.flush();
            handler->sock->sendData(serial_str);
        }
    }



    pthread_exit(NULL);
}


/**
 * get a new driver from the client, set him a car and send the cab to the
 * client.
 */
void GameFlow::recieveDrivers() {
    int numOfDrivers;
    cin >> numOfDrivers;
        char buffer[9999];
        //get the driverp

    for (int i=0; i<numOfDrivers;i++) {
        this->establishCommunication("TCP");
        this->comm->initialize();
        ClientHandler* handler = new ClientHandler(this->comm,
                                                   this->taxiCenter);
        pthread_create(&this->threads[i], NULL, test, (void*)handler);
        this->portNo++;
        pthread_join(this->threads[i], NULL);
    }
    /*
        for (int i=0; i<numOfDrivers;i++){
            this->establishCommunication("TCP");
            this->comm->initialize();
            this->sockVector.push_back(this->comm);

            this->comm->reciveData(buffer, sizeof(buffer));
            Driver *d2;
            boost::iostreams::basic_array_source<char> device(buffer,
                                                              sizeof(buffer));
            boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s(
                    device);
            boost::archive::binary_iarchive ia(s);
            ia >> d2;
            this->taxiCenter->addDriverInfo(d2);
            //send the cab
            Cab *cab = this->taxiCenter->getDriver()->getCab();
            string serial_str;
            boost::iostreams::back_insert_device<std::string> inserter(serial_str);
            boost::iostreams::stream<boost::iostreams::back_insert_device<std
            ::string> > stream(inserter);
            boost::archive::binary_oarchive oa(stream);
            oa << cab;
            stream.flush();
            comm->sendData(serial_str);
            this->portNo++;
        }
        */
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
        }
        else if (trips[i]->getPath()->empty()){
            trips[i]->getDriver()->setAvailable(true);
            taxiCenter->popTrip();
        }
    }
    time++;
    option = 10;
}

/**
 * after the clock has been moved, the client should get a new location from
 * the trip and set his current location
 */
void GameFlow::sendClientNewLocation() {
    Point *newLocation = this->taxiCenter->getDriver()->getCurrentLocation();
    string serial_str;
    boost::iostreams::back_insert_device<std::string> inserter(serial_str);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> >
            s(inserter);
    boost::archive::binary_oarchive oa(s);
    oa << newLocation;
    s.flush();
    comm->sendData(serial_str);
}

/**
 * after 7, we need to finish the program so send the client a null and it
 * will die too.
 */
void GameFlow::killTheClient() {
    Point *newLocation = NULL;
    string serial_str;
    boost::iostreams::back_insert_device<std::string> inserter(serial_str);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(
            inserter);
    boost::archive::binary_oarchive oa(s);
    oa << newLocation;
    s.flush();
    comm->sendData(serial_str);
}
    /*
Socket* GameFlow::getSock(){
    return this->comm;
}
     */