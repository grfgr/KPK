// Client-side effects. The broken KPK "explodes": a shrunken grenade flash, sparks of a short
// circuit, a small bang and an electric crack. Visual and sound only; the damage is done on the server.
class KPK_Fx
{
	static void SmallBlast(vector pos)
	{
		Particle blast = Particle.PlayInWorld(ParticleList.RGD5, pos);
		if (blast)
		{
			blast.ScaleParticleParamFromOriginal(EmitorParam.SIZE, 0.3);
			blast.ScaleParticleParamFromOriginal(EmitorParam.VELOCITY, 0.3);
			blast.ScaleParticleParamFromOriginal(EmitorParam.BIRTH_RATE, 0.4);
		}
		Particle.PlayInWorld(ParticleList.BARBED_WIRE_SPARKS, pos);

		EffectSound bang = SEffectManager.PlaySound("tire_small_explosion_SoundSet", pos);
		if (bang)
			bang.SetSoundAutodestroy(true);
		EffectSound crack = SEffectManager.PlaySound("defibrillator_shock_SoundSet", pos);
		if (crack)
			crack.SetSoundAutodestroy(true);
	}
};
