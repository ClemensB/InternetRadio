#include "MainWindow.hpp"

#include <process.h>
#include <CommCtrl.h>
#include <Uxtheme.h>

#include "OSUtil.hpp"

using namespace std;

namespace inetr {
	void MainWindow::radioOpenURL(string url) {
		void* *args = new void*[2];

		string *str = new string(url);
		*args = this;
		*(args + 1) = str;

		currentStreamURL = url;

		_beginthread(staticRadioOpenURLThread, 0, (void*)args);
	}

	void MainWindow::radioOpenURLThread(string url) {
		radioStatus_currentMetadata = "";

		KillTimer(window, bufferTimerId);
		KillTimer(window, metaTimerId);

		if (currentStream != 0) {
			BASS_ChannelStop(currentStream);
			BASS_StreamFree(currentStream);
		}

		radioStatus = Connecting;
		updateStatusLabel();

		HSTREAM tempStream = BASS_StreamCreateURL(url.c_str(), 0, 0, nullptr
			, 0);

		if (currentStreamURL != url) {
			BASS_StreamFree(tempStream);
			return;
		}

		currentStream = tempStream;

		if (currentStream != 0) {
			SetTimer(window, bufferTimerId, 50, nullptr);
		} else {
			radioStatus = ConnectionError;
			updateStatusLabel();
		}
	}

	void MainWindow::radioStop() {
		if (currentStream != 0) {
			BASS_ChannelStop(currentStream);
			BASS_StreamFree(currentStream);
		}

		ShowWindow(stationImg, SW_HIDE);
		radioStatus = Idle;
		updateStatusLabel();

		KillTimer(window, bufferTimerId);
		KillTimer(window, metaTimerId);

		currentStation = nullptr;
	}

	float MainWindow::radioGetVolume() const {
		return radioMuted ? 0.0f : radioVolume;
	}

	void MainWindow::radioSetVolume(float volume) {
		radioSetMuted(false);

		radioVolume = volume;
		if (currentStream)
			BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL,
			radioGetVolume());

		SendMessage(volumePbar, PBM_SETPOS, (WPARAM)(volume * 100.0f),
			(LPARAM)0);

		ShowWindow(volumePbar, SW_SHOW);
		SetTimer(window, hideVolBarTimerId, 1000, nullptr);
	}

	void MainWindow::radioSetMuted(bool muted) {
		radioMuted = muted;
		if (currentStream)
			BASS_ChannelSetAttribute(currentStream, BASS_ATTRIB_VOL,
			radioGetVolume());

		if (OSUtil::IsVistaOrLater()) {
			SendMessage(volumePbar, PBM_SETSTATE, muted ?
				(isColorblindModeEnabled ? PBST_PAUSED : PBST_ERROR) :
				PBST_NORMAL, (LPARAM)0);
		} else if (IsAppThemed() == FALSE) {
			SendMessage(volumePbar, PBM_SETBARCOLOR,
				(WPARAM)0, (LPARAM)(muted ? RGB(255, 0, 0) : CLR_DEFAULT));
		}

		ShowWindow(volumePbar, SW_SHOW);
		if (muted)
			KillTimer(window, hideVolBarTimerId);
		else
			SetTimer(window, hideVolBarTimerId, 1000, nullptr);

		updateStatusLabel();
	}
}