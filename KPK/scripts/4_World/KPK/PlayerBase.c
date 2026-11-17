modded class PlayerBase
{
	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type != KPK_RPC)
			return;

		Param2<int, string> data = new Param2<int, string>(0, "");
		if (!ctx.Read(data))
			return;

		if (GetGame().IsDedicatedServer())
		{
			// a client may only talk through its own player object
			if (!sender || !GetIdentity() || sender.GetId() != GetIdentity().GetId())
				return;
			if (KPK_Server.Get())
				KPK_Server.Get().OnRpc(this, sender, data.param1, data.param2);
		}
		else
		{
			KPK_Client.OnRpc(data.param1, data.param2);
		}
	}

	override void EEKilled(Object killer)
	{
		if (GetGame().IsDedicatedServer() && KPK_Server.Get())
			KPK_Server.Get().OnPlayerKilled(this, killer);

		super.EEKilled(killer);
	}
};

modded class ZombieBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);
		if (GetGame().IsDedicatedServer() && KPK_Server.Get())
			KPK_Server.Get().OnCreatureKilled(this, killer, false);
	}
};

modded class AnimalBase
{
	override void EEKilled(Object killer)
	{
		super.EEKilled(killer);
		if (GetGame().IsDedicatedServer() && KPK_Server.Get())
			KPK_Server.Get().OnCreatureKilled(this, killer, true);
	}
};
