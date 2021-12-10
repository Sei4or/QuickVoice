#include "pch.h"
#include "QuickVoice.h"

std::string QuickVoice::GetPluginName() {
	return "QuickVoice";
}

void QuickVoice::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

// Render the plugin settings here
// This will show up in bakkesmod when the plugin is loaded at
// f2 -> plugins -> QuickVoice
void QuickVoice::RenderSettings() {
	CVarWrapper enabledCvar = cvarManager->getCvar("qv_enabled");
	if (!enabledCvar) return;
	bool enabled = enabledCvar.getBoolValue();
	if (ImGui::Checkbox("Enabled", &enabled))
	{
		enabledCvar.setValue(enabled);
	}

	CVarWrapper preloadSoundsCvar = cvarManager->getCvar("qv_preload_sounds");
	if (!preloadSoundsCvar) return;
	bool preloadSounds = preloadSoundsCvar.getBoolValue();
	if (ImGui::Checkbox("Load all sounds on start-up", &preloadSounds))
	{
		preloadSoundsCvar.setValue(preloadSounds);
	}
	if (ImGui::IsItemHovered())
	{
		ImGui::SetTooltip("If checked this will pre-load all the sounds for quicker initial playback.\nThis will take up more memory, but won't be reading from the disk when a new sound is loaded.");
	}

	CVarWrapper volumeCvar = cvarManager->getCvar("qv_volume");
	if (!volumeCvar) return;
	float volume = volumeCvar.getFloatValue();
	if (ImGui::SliderFloat("Volume", &volume, 0.01, 5.0))
	{
		volumeCvar.setValue(volume);
	}
}
