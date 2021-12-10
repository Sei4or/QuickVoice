#include "pch.h"
#include "QuickVoice.h"
#include "sound_interface.h"

#include <windows.h>
#include <mmsystem.h>
#include <xaudio2.h>

BAKKESMOD_PLUGIN(QuickVoice, "QuickVoice", plugin_version, PLUGINTYPE_FREEPLAY)

std::shared_ptr<CVarManagerWrapper> _globalCvarManager;
std::shared_ptr<GameWrapper> _globalGameWrapper;

struct ChatMessage
{
	void* PRI;
	void* Team;
	wchar_t* PlayerName;
	uint8_t PlayerNamePadding[0x8];
	wchar_t* Message;
	uint8_t MessagePadding[0x8];
	uint8_t ChatChannel;
	unsigned long bPreset : 1;
};

void QuickVoice::hookChatMessageEvent()
{
	gameWrapper->HookEventWithCaller<ActorWrapper>("Function TAGame.HUDBase_TA.OnChatMessage",
		[this](ActorWrapper caller, void* params, std::string eventName) {
			if (params)
			{
				ChatMessage* chatMessage = static_cast<ChatMessage*>(params);
				std::wstring playerName(chatMessage->PlayerName);
				std::wstring message(chatMessage->Message);
				std::string bMessage(message.begin(), message.end());
				cvarManager->log("Message: " + bMessage);

				if (bMessage == "Group2Message4" || bMessage == "What a save!") {
					SoundInterface::playSound(WHAT_A_SAVE);
				}
				else if (bMessage == "Group2Message1" || bMessage == "Nice shot!") {
					SoundInterface::playSound(NICE_SHOT);
				}
				else if (bMessage == "Group2Message3" || bMessage == "Thanks!") {
					SoundInterface::playSound(THANKS);
				}
				else if (bMessage == "e" || bMessage == "E") {
					SoundInterface::playSound(E);
				}
				else if (bMessage == "Group1Message1" || bMessage == "I got it!")
				{
					SoundInterface::playSound(I_GOT_IT);
				}
				else if (bMessage == "Group1Message2" || bMessage == "Need boost!")
				{
					SoundInterface::playSound(NEED_BOOST);
				}
			}
		});
}

void QuickVoice::unHookChatMessageEvent()
{
	gameWrapper->UnhookEvent("Function TAGame.HUDBase_TA.OnChatMessage");
}

void QuickVoice::onLoad()
{
	_globalCvarManager = cvarManager;
	_globalGameWrapper = gameWrapper;

	enabled = std::make_shared<bool>(true);
	CVarWrapper enabledCvar = cvarManager->getCvar("qv_enabled");
	if (!enabledCvar)
	{
		enabledCvar = cvarManager->registerCvar("qv_enabled", "1");
	}
	enabledCvar.bindTo(enabled);
	if (enabled) this->hookChatMessageEvent();
	enabledCvar.addOnValueChanged([this](std::string oldValue, CVarWrapper newCvar) {
		if (newCvar.getBoolValue())
		{
			this->hookChatMessageEvent();
		}
		else
		{
			this->unHookChatMessageEvent();
		}
		});

	CVarWrapper preloadSoundsCvar = cvarManager->getCvar("qv_preload_sounds");
	if (!preloadSoundsCvar)
	{
		preloadSoundsCvar = cvarManager->registerCvar("qv_preload_sounds", "0");
	}
	preloadSoundsCvar.addOnValueChanged([this](std::string oldValue, CVarWrapper newCvar) {
		if (newCvar.getBoolValue())
		{
			cvarManager->log("Preloading sounds");
			SoundInterface::preloadSounds();
		}
	});

	CVarWrapper volumeCvar = cvarManager->getCvar("qv_volume");
	if (!volumeCvar)
	{
		volumeCvar = cvarManager->registerCvar("qv_volume", "1.0");
	}
}

void QuickVoice::onUnload()
{
	// SoundInterface::unload();
}