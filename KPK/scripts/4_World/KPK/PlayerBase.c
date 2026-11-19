modded class PlayerBase
{
	override void SetActionsRemoteTarget(out TInputActionMap InputActionMap)
	{
		super.SetActionsRemoteTarget(InputActionMap);
		AddAction(ActionKPKWorldContact, InputActionMap);
		AddAction(ActionKPKWorldInvite, InputActionMap);
	}

	override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
	{
		super.OnRPC(sender, rpc_type, ctx);

		if (rpc_type == KPK_RPC_FX && !GetGame().IsDedicatedServer())
		{
			Param1<vector> fx = new Param1<vector>(vector.Zero);
			if (ctx.Read(fx))
				KPK_Fx.SmallBlast(fx.param1);
			return;
		}

		if (rpc_type == KPK_RPC_RADIO && !GetGame().IsDedicatedServer())
		{
			Param2<bool, int> radio = new Param2<bool, int>(false, 0);
			if (ctx.Read(radio))
				KPK_Device.RadioClient(this, radio.param1, radio.param2);
			return;
		}

		if (rpc_type != KPK_RPC)
			return;

		Param5<int, int, int, int, string> data = new Param5<int, int, int, int, string>(0, 0, 0, 0, "");
		if (!ctx.Read(data))
			return;

		if (GetGame().IsDedicatedServer())
		{
			// a client may only talk through its own player object
			if (!sender || !GetIdentity() || sender.GetId() != GetIdentity().GetId())
				return;
			if (KPK_Server.Get())
				KPK_Server.Get().OnChunk(this, sender, data.param1, data.param2, data.param3, data.param4, data.param5);
		}
		else
		{
			string json;
			if (KPK_Client.s_Assembler.Add(data.param1, data.param2, data.param3, data.param4, data.param5, json))
				KPK_Client.OnRpc(data.param1, json);
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
