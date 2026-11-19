// Interface sounds of the KPK (config CfgSoundSets KPK_*_SoundSet), played only on this client at the
// camera: key clicks, the module switch blip and, now and then while the KPK is open, a beep, a data
// chirp or a burst of static (the Monolith device whispers instead).
class KPK_Sound
{
	static void Play(string name)
	{
		if (!GetGame() || GetGame().IsDedicatedServer())
			return;
		EffectSound snd = SEffectManager.PlaySound("KPK_" + name + "_SoundSet", GetGame().GetCurrentCameraPosition());
		if (snd)
			snd.SetSoundAutodestroy(true);
	}

	// soft white noise of the speaker while the KPK is open (looped; stop it with StopHiss)
	static EffectSound StartHiss()
	{
		if (!GetGame() || GetGame().IsDedicatedServer())
			return null;
		return SEffectManager.PlaySound("KPK_hiss_SoundSet", GetGame().GetCurrentCameraPosition(), 0.4, 0.3, true);
	}

	static void StopHiss(EffectSound snd)
	{
		if (!snd)
			return;
		snd.SetSoundAutodestroy(true);
		snd.SoundStop();
	}

	static void Ambient(bool monolith)
	{
		int roll = Math.RandomInt(0, 4);
		string name = "static";
		if (monolith)
		{
			if (roll == 1 || roll == 2)
				name = "whisper";
			else if (roll == 3)
				name = "data";
		}
		else
		{
			if (roll == 0)
				name = "beep1";
			else if (roll == 1)
				name = "beep2";
			else if (roll == 2)
				name = "data";
		}
		Play(name);
	}
};
