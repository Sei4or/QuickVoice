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
				if (chatMessage->PlayerName == nullptr) return;
				std::wstring playerName(chatMessage->PlayerName);
				if (chatMessage->Message == nullptr) return;
				std::wstring message(chatMessage->Message);
				std::string bMessage(message.begin(), message.end());
				// cvarManager->log("Message: " + bMessage);

				if (SoundInterface::quickChatIds.find(bMessage) != SoundInterface::quickChatIds.end())
				{
					HRESULT hr;
					if (FAILED(hr = this->soundManager.playSound(SoundInterface::quickChatIds.at(bMessage))))
					{
						LOG("Failed to play sound ({}). HRESULT: {}", bMessage, hr);
					}
				}
			}
		}
	);
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
	if (enabled)
	{
		this->hookChatMessageEvent();
		HRESULT hr;
		if (FAILED(hr = this->soundManager.initialize()))
		{
			LOG("Failed to initialize the sound manager. HRESULT: {}", hr);
		}
	}
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
			soundManager.preloadSounds();
		}
	});

	CVarWrapper volumeCvar = cvarManager->getCvar("qv_volume");
	if (!volumeCvar)
	{
		volumeCvar = cvarManager->registerCvar("qv_volume", "1.0");
	}
	volumeCvar.addOnValueChanged([this](std::string oldValue, CVarWrapper newCvar) {
		HRESULT hr;
		if (FAILED(hr = this->soundManager.setVolume(newCvar.getFloatValue())))
		{
			LOG("Failed to set volume to {}. HRESULT: {}", newCvar.getFloatValue(), hr);
		}
	});
}

void QuickVoice::onUnload()
{
	this->unHookChatMessageEvent();
	this->soundManager.unload();
}