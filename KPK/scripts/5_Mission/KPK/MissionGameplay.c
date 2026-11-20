modded class MissionGameplay
{
	protected ref KPK_WorldLabels m_KPKLabels;

	override void OnInit()
	{
		super.OnInit();
		m_KPKLabels = new KPK_WorldLabels();
		KPK_Client.s_OnNotify.Insert(KPK_Toast.Show);
		KPK_Client.s_OnSound.Insert(KPK_Sound.Play);
		KPK_Client.s_OnOpen.Insert(KPK_OnOpen);
		KPK_Client.s_OnDenied.Insert(KPK_OnDenied);
	}

	override void OnMissionFinish()
	{
		KPK_Client.s_OnOpen.Remove(KPK_OnOpen);
		KPK_Client.s_OnDenied.Remove(KPK_OnDenied);
		KPK_Client.s_OnNotify.Remove(KPK_Toast.Show);
		KPK_Client.s_OnSound.Remove(KPK_Sound.Play);
		KPK_Toast.Clear();
		KPK_Menu.Shutdown();
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
					KPK_Client.Toast("КПК", "Метки группы: включены", 2, false);
				else
					KPK_Client.Toast("КПК", "Метки группы: выключены", 2, false);
			}
			else if (toggle && toggle.LocalPress())
			{
				if (KPK_Device.FindOn(player))
					KPK_Client.RequestOpen();
				else
					KPK_Client.Toast("КПК", "У вас нет КПК", 3, false);
			}
		}

		if (m_KPKLabels)
			m_KPKLabels.Update(timeslice);
		KPK_Toast.Update(timeslice);
	}

	void KPK_OnOpen()
	{
		if (!KPK_Menu.IsOpen() && !GetGame().GetUIManager().GetMenu())
			KPK_Menu.Open();
	}

	void KPK_OnDenied(string reason)
	{
		KPK_Menu.CloseIfOpen();
		KPK_Client.Toast("КПК", reason, 4, true);
	}
};
