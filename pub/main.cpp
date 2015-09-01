//
// 264-toy
//

#include <iostream>

#include "pub.hpp"

using namespace std;

int __generic_main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	__generic_main(argc, argv);

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
