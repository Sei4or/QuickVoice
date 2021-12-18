#pragma once

#include "SourceVoiceManager.h"

#include <xaudio2.h>
#include <memory>
#include <unordered_map>

#define I_GOT_IT 0 // Group1Message1
#define NEED_BOOST 1 // Group1Message2
#define TAKE_THE_SHOT 2 // Group1Message3
#define DEFENDING 3 // Group1Message4
#define GO_FOR_IT 4 // Group1Message5
#define CENTERING 5 // Group1Message6
#define ALL_YOURS 6 // Group1Message7
#define IN_POSITION 7 // Group1Message8
#define INCOMING 8 // Group1Message9
#define FAKING 9 // Group1Message10
#define BUMPING 10 // Group1Message11
#define ON_YOUR_LEFT 11 // Group1Message12
#define ON_YOUR_RIGHT 12 // Group1Message13
#define PASSING 13 // Group1Message14

#define NICE_SHOT 14 // Group2Message1
#define GREAT_PASS 15 // Group2Message2
#define THANKS 16 // Group2Message3
#define WHAT_A_SAVE 17 // Group2Message4
#define NICE_ONE 18 // Group2Message5
#define WHAT_A_PLAY 19 // Group2Message6
#define GREAT_CLEAR 20 // Group2Message7
#define NICE_BLOCK 21 // Group2Message8
#define NICE_BUMP 22 // Group2Message9
#define NICE_DEMO 23 // Group2Message10

#define OMG 24 // Group3Message1
#define NOOOO 25 // Group3Message2
#define WOW 26 // Group3Message3
#define CLOSE_ONE 27 // Group3Message4
#define NO_WAY 28 // Group3Message5
#define HOLY_COW 29 // Group3Message6
#define WHEW 30 // Group3Message7
#define SIIIICK 31 // Group3Message8
#define CALCULATED 32 // Group3Message9
#define SAVAGE 33 // Group3Message10
#define OKAY 34 // Group3Message11

#define CURSING 35 // Group4Message1 ($#@%)
#define NO_PROBLEM 36 // Group4Message2
#define WHOOPS 37 // Group4Message3
#define SORRY 38 // Group4Message4
#define MY_BAD 39 // Group4Message5
#define OOPS 40 // Group4Message6
#define MY_FAULT 41 // Group4Message7

#define GG 42 // Group5Message1
#define WELL_PLAYED 43 // Group5Message2
#define THAT_WAS_FUN 44 // Group5Message3
#define REMATCH 45 // Group5Message4
#define ONE_MORE_GAME 46 // Group5Message5
#define WHAT_A_GAME 47 // Group5Message6
#define NICE_MOVES 48 // Group5Message7
#define EVERYBODY_DANCE 49 // Group5Message8

namespace SoundInterface
{
	static std::unordered_map<std::string, short int> quickChatIds = {
		{"Group1Message1", 0},
		{"Group1Message2", 1},
		{"Group1Message3", 2},
		{"Group1Message4", 3},
		{"Group1Message5", 4},
		{"Group1Message6", 5},
		{"Group1Message7", 6},
		{"Group1Message8", 7},
		{"Group1Message9", 8},
		{"Group1Message10", 9},
		{"Group1Message11", 10},
		{"Group1Message12", 11},
		{"Group1Message13", 12},
		{"Group1Message14", 13},
		{"Group2Message1", 14},
		{"Group2Message2", 15},
		{"Group2Message3", 16},
		{"Group2Message4", 17},
		{"Group2Message5", 18},
		{"Group2Message6", 19},
		{"Group2Message7", 20},
		{"Group2Message8", 21},
		{"Group2Message9", 22},
		{"Group2Message10", 23},
		{"Group3Message1", 24},
		{"Group3Message2", 25},
		{"Group3Message3", 26},
		{"Group3Message4", 27},
		{"Group3Message5", 28},
		{"Group3Message6", 29},
		{"Group3Message7", 30},
		{"Group3Message8", 31},
		{"Group3Message9", 32},
		{"Group3Message10", 33},
		{"Group3Message11", 34},
		{"Group4Message1", 35},
		{"Group4Message2", 36},
		{"Group4Message3", 37},
		{"Group4Message4", 38},
		{"Group4Message5", 39},
		{"Group4Message6", 40},
		{"Group4Message7", 41},
		{"Group5Message1", 42},
		{"Group5Message2", 43},
		{"Group5Message3", 44},
		{"Group5Message4", 45},
		{"Group5Message5", 46},
		{"Group5Message6", 47},
		{"Group5Message7", 48},
		{"Group5Message8", 49}
	};

	class SoundManager
	{
	public:
		HRESULT initialize();
		HRESULT loadSound(short int soundId);
		HRESULT playSound(short int soundId);
		void preloadSounds();
		void unloadSounds();
		void unload();
		HRESULT setVolume(float newVolume);
	private:
		std::shared_ptr<IXAudio2> pXAudio2;
		std::shared_ptr<IXAudio2MasteringVoice> pMasterVoice;
		std::shared_ptr<WAVEFORMATEXTENSIBLE> wfx;
		std::unordered_map<short int, XAUDIO2_BUFFER> loadedSounds;
		SourceVoiceManager sourceVoiceManager;
	};
}