#include <Windows.h>
#include <iostream>
#include "826api.h"


using namespace std;

int SetDacOutput(uint board, uint chan, uint range, double volts);

int SetDacOutput(uint board, uint chan, uint range, double volts)
{
	uint setpoint;
	int errcode;

	switch (range)
	{
	case S826_DAC_SPAN_0_5:
		setpoint = (uint)(volts * 0xFFFF / 5);
		break;
	case S826_DAC_SPAN_0_10:
		setpoint = (uint)(volts * 0xFFFF / 10);
		break;
	case S826_DAC_SPAN_5_5:
		setpoint = (uint)(volts * 0xFFFF / 10) + 0x8000;
		break;
	case S826_DAC_SPAN_10_10:
		setpoint = (uint)(volts * 0xFFFF / 20) + 0x8000;
		break;
	default:
		break;
	}

	errcode = S826_DacDataWrite(board, chan, setpoint, 0);  // program DAC output

	return errcode;
}

int main(int argc, char* argv[])
{
	// Total number of analog IO on the card
	const uint NUM_AIO = 8;

	// Error code returned by the S826 API functions
	int errcode = S826_ERR_OK;

	// Board configuration
	uint board = 0;
	int boardflags = S826_SystemOpen();

	if (boardflags < 0)
		errcode = boardflags;					// problem during open
	else if ((boardflags & (1 << board)) == 0)
	{
		int i;
		cout << "Target board of index " << board << " not found" << endl;         // driver didn't find board you want to use
		for (i = 0; i < 8; i++) {
			if (boardflags & (1 << i)) {
				cout << "board " << i << " detected." << endl;
			}
		}
	}

	cout << "Setting analog outputs to zero volts." << endl;

	for (uint channel = 0; channel < NUM_AIO; ++channel)
	{
		// Read the output range
		uint range, setpoint;
		errcode = S826_DacRead(board, channel, &range, &setpoint, 0);

		// Print output range of the channel
		cout << "Channel " << channel << ", output range is set to " << range << endl;

		// Set ouput to 0V
		if (errcode == S826_ERR_OK)
		{
			cout << "\tSetting channel " << channel << " to 0V" << endl;
			errcode = SetDacOutput(board, channel, range, 0);
		}
		else
			break;
	}

	cout << "All analog output channels are set to 0V." << endl;

	switch (errcode)
	{
	case S826_ERR_OK:
		break;
	case S826_ERR_BOARD:
		cout << "Illegal board number" << endl;
		break;
	case S826_ERR_VALUE:
		cout << "Illegal argument" << endl;
		break;
	case S826_ERR_NOTREADY:
		cout << "Device not ready or timeout" << endl;
		break;
	case S826_ERR_CANCELLED:
		cout << "Wait cancelled" << endl;
		break;
	case S826_ERR_DRIVER:
		cout << "Driver call failed" << endl;
		break;
	case S826_ERR_MISSEDTRIG:
		cout << "Missed adc trigger" << endl;
		break;
	case S826_ERR_DUPADDR:
		cout << "Two boards have same number" << endl;
		break;
		S826_SafeWrenWrite(board, 0x02);
	case S826_ERR_BOARDCLOSED:
		cout << "Board not open" << endl;
		break;
	case S826_ERR_CREATEMUTEX:
		cout << "Can't create mutex" << endl;
		break;
	case S826_ERR_MEMORYMAP:
		cout << "Can't map board" << endl;
		break;
	default:
		cout << "Unknown error" << endl;
		break;
	}

	cout << "Press enter to continue..." << endl;
	cin.ignore();

	// Close connection to the card
	S826_SystemClose();

	return 0;
}