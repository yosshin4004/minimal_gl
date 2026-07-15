/* Copyright (C) 2026 Yosshin(@yosshin4004) */
#include "config.h"
#include "common.h"
#include "midi_state_tracker.h"
#include <windows.h>
#include <mmsystem.h>

static struct {
	HMIDIIN	handle;
	bool	isAvailable;
	uint8_t	states[256];
} s_midiIn = {};


static void CALLBACK MidiInProc(
	HMIDIIN hMidiIn,
	UINT wMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2
){
	if (wMsg == MIM_DATA) {
		int32_t msg = (int32_t)dwParam1;

		uint8_t status = msg & 0xff;
		if (
			(status & 0xF0) == 0x90		/* Note on */
		||	(status & 0xF0) == 0xB0		/* Control Change */
		) {
			uint8_t data1  = (msg >> 8) & 0xff;
			uint8_t data2  = (msg >> 16) & 0xff;
#if 0
			printf(
				"status=%02X data1=%d data2=%d\n",
				status, data1, data2
			);
#endif
			s_midiIn.states[data1] = data2;
		}
	}
}


uint8_t MidiStateGet(uint8_t ccNumber){
	return s_midiIn.states[ccNumber & 255];
}


bool MidiStateTrackerInitialize(){
	printf("setting up MIDI state tracker ...\n");

	int numDevs = midiInGetNumDevs();
	for (int i = 0; i < numDevs; ++i) {
		MIDIINCAPS caps = {};
		midiInGetDevCaps(i, &caps, sizeof(caps));
		printf("MIDI device #%d: %s\n", i, caps.szPname);
	}

	MMRESULT ret = midiInOpen(
		/* LPHMIDIIN phmi */		&s_midiIn.handle,
		/* UINT      uDeviceID */	0,              /* デバイス番号 */
		/* DWORD_PTR dwCallback */	(DWORD_PTR)MidiInProc,
		/* DWORD_PTR dwInstance */	0,
		/* DWORD     fdwOpen */		CALLBACK_FUNCTION
	);
	s_midiIn.isAvailable = (ret == MMSYSERR_NOERROR);
	if (s_midiIn.isAvailable == false) {
		printf("setting up MIDI state tracker ... device not found.\n");
	    return false;
	}

	midiInStart(s_midiIn.handle);

	printf("setting up MIDI state tracker ... done.\n");
    return true;
}

bool MidiStateTrackerTerminate(){
	if (s_midiIn.isAvailable) {
		midiInStop(s_midiIn.handle);
		midiInClose(s_midiIn.handle);
	}

    return true;
}

