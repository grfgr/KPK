modded class MissionGameplay
{
	protected ref KPK_WorldLabels m_KPKLabels;

	override void OnInit()
	{
		super.OnInit();
		m_KPKLabels = new KPK_WorldLabels();
		KPK_Client.s_OnOpen.Insert(KPK_OnOpen);
		KPK_Client.s_OnDenied.Insert(KPK_OnDenied);
	}

	override void OnMissionFinish()
	{
		KPK_Client.s_OnOpen.Remove(KPK_OnOpen);
		KPK_Client.s_OnDenied.Remove(KPK_OnDenied);
		KPK_Menu.CloseIfOpen();
		m_KPKLabels = null;
		super.OnMissionFinish();
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
		if (player && player.IsAlive() && !KPK_Menu.IsOpen() && !KPK_Menu.JustClosed() && !GetGame().GetUIManager().GetMenu())
		{
			UAInput toggle = GetUApi().GetInputByName("UAKPKToggle");
			UAInput labels = GetUApi().GetInputByName("UAKPKLabels");
			if (labels && labels.LocalPress())
			{
				KPK_Client.s_LabelsEnabled = !KPK_Client.s_LabelsEnabled;
				if (KPK_Client.s_LabelsEnabled)
					NotificationSystem.AddNotificationExtended(2, "КПК", "Метки группы: включены", "set:dayz_gui image:icon");
				else
					NotificationSystem.AddNotificationExtended(2, "КПК", "Метки группы: выключены", "set:dayz_gui image:icon");
			}
			else if (toggle && toggle.LocalPress())
			{
				if (KPK_Device.FindOn(player))
					KPK_Client.RequestOpen();
				else
					NotificationSystem.AddNotificationExtended(3, "КПК", "У вас нет КПК", "set:dayz_gui image:icon");
			}
		}

		if (m_KPKLabels)
			m_KPKLabels.Update(timeslice);
	}

	void KPK_OnOpen()
	{
		if (!KPK_Menu.IsOpen() && !GetGame().GetUIManager().GetMenu())
			KPK_Menu.Open();
	}

	void KPK_OnDenied(string reason)
	{
		KPK_Menu.CloseIfOpen();
		NotificationSystem.AddNotificationExtended(4, "КПК", reason, "set:dayz_gui image:icon");
	}
};
