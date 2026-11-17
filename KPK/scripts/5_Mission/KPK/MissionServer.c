modded class MissionServer
{
	override void OnInit()
	{
		super.OnInit();
		KPK_Server.Start();
	}

	override void OnClientDisconnectedEvent(PlayerIdentity identity, PlayerBase player, int logoutTime, bool authFailed)
	{
		if (identity && KPK_Server.Get())
			KPK_Server.Get().OnClientDisconnected(identity.GetPlainId());
		super.OnClientDisconnectedEvent(identity, player, logoutTime, authFailed);
	}

	override void OnMissionFinish()
	{
		KPK_Server.Stop();
		super.OnMissionFinish();
	}
};
