//
// 264-toy
//

#include <iostream>

#include "pub.hpp"

using namespace std;

int __pili_main(int argc, char *argv[]);
int __generic_main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
#if defined __PILI_SDK__
	__pili_main(argc, argv);
#else
	__generic_main(argc, argv);
#endif

	return 0;
}

void PiliStreamCallback(uint8_t state)
{
	cout << "state changed :" << (int)state << endl;
}

int __pili_main(int argc, char *argv[])
{
	RtmpStream rtmp("pili");

	if (rtmp.PiliConnect(argv[1], PiliStreamCallback)) {
		rtmp.PiliSendH264File(argv[2]);
	}
	rtmp.PiliDisconnect();
	cout << "Pili: publish is done" << endl;

	return 0;
}

int __generic_main(int argc, char *argv[])
{
	RtmpStream rtmp;

	if (argc != 3) {
		cout << "arguments not match! arg0="<< argv[0] << endl;
		exit(1);
	}

	if (rtmp.Connect(argv[1]) == true) {
		rtmp.SendH264File(argv[2]);
	} else {
		cout << "Not connected !" << endl;
	}
	rtmp.Close();

	return 0;
}
