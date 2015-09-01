//
// 264-toy
//

#include <iostream>

#include "pili_pub.hpp"

using namespace std;

int __pili_main(int argc, char *argv[]);

int main(int argc, char *argv[])
{
	__pili_main(argc, argv);

	return 0;
}

void PiliStreamCallback(uint8_t state)
{
	cout << "state changed :" << (int)state << endl;
}

int __pili_main(int argc, char *argv[])
{
	PiliStream rtmp;

	if (rtmp.Connect(argv[1], PiliStreamCallback)) {
		rtmp.SendH264File(argv[2]);
	}
	rtmp.Close();
	cout << "Pili: publish is done" << endl;

	return 0;
}

