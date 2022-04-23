#include "Audio.h"
#include "Helpers/Util.h"

std::unordered_map<std::string, SoLoud::Wav*> Audio::s_loadedAudio;
SoLoud::Soloud Audio::gSoloud;

void Audio::Init()
{
	gSoloud.init();

	Audio::LoadAudio("BodyFall_01.wav");
	Audio::LoadAudio("Shotgun_Fire_01.wav");
	Audio::LoadAudio("Door_Open.wav");
	Audio::LoadAudio("Door_Open2.wav");
	Audio::LoadAudio("player_step_1.wav");
	Audio::LoadAudio("player_step_2.wav");
	Audio::LoadAudio("player_step_3.wav");
	Audio::LoadAudio("player_step_4.wav");
	Audio::LoadAudio("Impact_Flesh.wav");
	Audio::LoadAudio("Empty.wav");
	Audio::LoadAudio("Shotgun_Reload_01.wav");
	Audio::LoadAudio("Shotgun_Reload_05.wav");
	Audio::LoadAudio("Shotgun_ReloadStart.wav");
	Audio::LoadAudio("Shotgun_ReloadEnd.wav");
	Audio::LoadAudio("ShellFloorBounce.wav");
	Audio::LoadAudio("ShellWallBounce.wav");
	Audio::LoadAudio("Glock_Fire_0.wav");
	Audio::LoadAudio("Glock_Fire_1.wav");
	Audio::LoadAudio("Glock_Fire_2.wav");
	Audio::LoadAudio("Glock_DryFire.wav");;
	Audio::LoadAudio("Glock_Reload.wav");
	Audio::LoadAudio("Glock_ReloadFromEmpty.wav");
	Audio::LoadAudio("Glock_DryFire.wav");
	Audio::LoadAudio("Glock_Equip.wav");
	Audio::LoadAudio("FLY_Bullet_Impact_Flesh_01.wav");
	Audio::LoadAudio("FLY_Bullet_Impact_Flesh_02.wav");
	Audio::LoadAudio("FLY_Bullet_Impact_Flesh_03.wav");
	Audio::LoadAudio("FLY_Bullet_Impact_Flesh_04.wav");
	Audio::LoadAudio("FLY_Bullet_Impact_Flesh_05.wav");
	Audio::LoadAudio("FLY_Bullet_Impact_Flesh_06.wav");
	Audio::LoadAudio("FLY_Bullet_Impact_Flesh_07.wav");
	Audio::LoadAudio("FLY_Bullet_Impact_Flesh_08.wav");
	Audio::LoadAudio("BuolletCasingBounce.wav");
	Audio::LoadAudio("FLY_Head_Explode_01.wav");
	Audio::LoadAudio("GlassImpact.wav");
	Audio::LoadAudio("UI_Beep.wav");
	Audio::LoadAudio("UI_Select.wav");
	Audio::LoadAudio("UI_Select2.wav");
	Audio::LoadAudio("UI_Select3.wav");
	Audio::LoadAudio("UI_Select4.wav");
	Audio::LoadAudio("UI_Select5.wav");
	Audio::LoadAudio("UI_Cursor.wav");
	Audio::LoadAudio("BulbBreak.wav");
	Audio::LoadAudio("Death0.wav");
	Audio::LoadAudio("Death1.wav");
	Audio::LoadAudio("Death2.wav");
	Audio::LoadAudio("Death3.wav");
	Audio::LoadAudio("Door_Latch.wav");
	Audio::LoadAudio("Shotgun_Fire_01.wav");
	
	//Audio::LoadAudio("Music.wav");

	Audio::LoadAudio("Music.wav");
}

void Audio::LoadAudio(const char* name)
{
	std::string fullpath = "res/audio/";
	fullpath += name;

	SoLoud::Wav* audio = new SoLoud::Wav();
	audio->load(fullpath.c_str());

	s_loadedAudio[name] = audio;
}

SoLoud::handle Audio::PlayAudio(const char* name, float volume)
{
	auto audio = s_loadedAudio[name];
	SoLoud::handle handle = gSoloud.play(*audio, volume);
	return handle;
}

SoLoud::handle Audio::LoopAudio(const char* name, float volume)
{
	auto audio = s_loadedAudio[name];
	SoLoud::handle handle = gSoloud.play(*audio, volume);
	audio->setLooping(true);

	return handle;
	//std::cout << "AUDIO NOT FOUND: " << name << "\n";
}


void Audio::Terminate()
{
	gSoloud.deinit();
}

void Audio::PlayGlockFlesh()
{
	int RandomAudio = rand() % 8;
	if (RandomAudio == 0)
		Audio::PlayAudio("FLY_Bullet_Impact_Flesh_01.wav");
	if (RandomAudio == 1)
		Audio::PlayAudio("FLY_Bullet_Impact_Flesh_02.wav");
	if (RandomAudio == 2)
		Audio::PlayAudio("FLY_Bullet_Impact_Flesh_03.wav");
	if (RandomAudio == 3)
		Audio::PlayAudio("FLY_Bullet_Impact_Flesh_04.wav");
	if (RandomAudio == 4)
		Audio::PlayAudio("FLY_Bullet_Impact_Flesh_05.wav");
	if (RandomAudio == 5)
		Audio::PlayAudio("FLY_Bullet_Impact_Flesh_06.wav");
	if (RandomAudio == 6)
		Audio::PlayAudio("FLY_Bullet_Impact_Flesh_07.wav");
	if (RandomAudio == 7)
		Audio::PlayAudio("FLY_Bullet_Impact_Flesh_08.wav");
}