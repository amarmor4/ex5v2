#include <iostream>
#include "Driver.h"
#include "Udp.h"
#include "Tcp.h"

using namespace std;
using namespace boost::iostreams;
using namespace boost::archive;

int main(int argc, char *argv[]) {
    int id, age, experience, cabId;
    Status status;
    char c, statusLetter;
    Driver *driver;

    //get an input from the user and set a new driver
    cin >> id >> c >> age >> c >> statusLetter >> c >>
        experience >> c >> cabId;
    status = Status(statusLetter);
    driver = new Driver(id, age, status, experience, cabId);

    // create new socket
    //Socket *udp = new Udp(0, atoi(argv[2]));
    Socket *sock=new Tcp(0,atoi(argv[2]));
    sock->initialize();
    char buffer[9999];

    // serial driver object to string
    string serial_str;
    back_insert_device<std::string> inserter(serial_str);
    stream<boost::iostreams::back_insert_device<std::string>> s(inserter);
    binary_oarchive oa(s);
    oa << driver;
    s.flush();

    // send driver object to server
    sock->sendData(serial_str, 0);
    // get a cab

    // get a cab serial string from server and add it to driver.
    sock->reciveData(buffer, sizeof(buffer), 0);
    Cab* cab;
    boost::iostreams::basic_array_source<char> device(buffer, sizeof(buffer));
    boost::iostreams::stream<boost::iostreams::basic_array_source<char> > stream1
            (device);
    boost::archive::binary_iarchive ia(stream1);
    ia >> cab;
    //cout << cab->getCabId() <<endl;
    driver->setCab(cab);


    //get from server point of the next step at path
    int b=1;
    while(b!=0){
        b=driver->setDataDriver(sock);
    }
    delete(driver);
    delete(cab);
    delete (sock);
    return 0;
}