//
// pub_mon
//

#define __PRINT_TABLE__

#include "pub_mon.hpp"

using namespace std;

int main(int argc, char** argv)
{
        if (argc != 2) {
                cout << "Error: args not match" << endl;
                exit(1);
        }

        PubMonitor conf;
        int nStatus = conf.LoadConfig(argv[1]);
        if (nStatus <= 0) {
                cout << "Error: config file not loaded" << endl;
                exit(1);
        }

        cout << "Info: " << nStatus << " entrie(s) loaded" << endl;
#ifdef __PRINT_TABLE__
        conf.PrintTable();
#endif
        conf.Run();
}
