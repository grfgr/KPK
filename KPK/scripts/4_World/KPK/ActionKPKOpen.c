class ActionKPKOpen : ActionSingleUseBase
{
	void ActionKPKOpen()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_PRESS_TRIGGER;
		m_CommandUIDProne = DayZPlayerConstants.CMD_ACTIONFB_PRESS_TRIGGER;
		m_Text = "Включить КПК";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTNone();
	}

	override bool HasTarget()
	{
		return false;
	}

	override bool HasProneException()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		return KPK_Device.IsKPK(item);
	}

	override void OnExecuteClient(ActionData action_data)
	{
		super.OnExecuteClient(action_data);
		KPK_Client.RequestOpen();
	}
};

// Broken KPK: switching it on (battery inserted) shows one random grid square from its memory,
// then it burns out for good: the device is ruined, the battery drained, the player takes 10 damage.
class ActionKPKBroken : ActionSingleUseBase
{
	void ActionKPKBroken()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_PRESS_TRIGGER;
		m_CommandUIDProne = DayZPlayerConstants.CMD_ACTIONFB_PRESS_TRIGGER;
		m_Text = "Включить разбитый КПК";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINonRuined();
		m_ConditionTarget = new CCTNone();
	}

	override bool HasTarget()
	{
		return false;
	}

	override bool HasProneException()
	{
		return true;
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		return KPK_Device.IsBroken(item) && !item.IsRuined();
	}

	override void OnExecuteServer(ActionData action_data)
	{
		super.OnExecuteServer(action_data);
		if (KPK_Server.Get())
			KPK_Server.Get().OnBrokenSwitchedOn(action_data.m_Player, action_data.m_MainItem);
	}
};

modded class ActionConstructor
{
	override void RegisterActions(TTypenameArray actions)
	{
		super.RegisterActions(actions);
		actions.Insert(ActionKPKOpen);
		actions.Insert(ActionKPKBroken);
		actions.Insert(ActionKPKWorldContact);
		actions.Insert(ActionKPKWorldInvite);
	}
};

// Looking at another player: give him your contact / invite him to the party. Both must be logged in with
// a working full KPK of their own in the hands; the other one confirms in his KPK.
class ActionKPKWorldContact : ActionInteractBase
{
	protected bool m_Invite;

	void ActionKPKWorldContact()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text = "КПК: передать свой контакт";
	}

	override void CreateConditionComponents()
	{
		m_ConditionItem = new CCINone();
		m_ConditionTarget = new CCTMan(UAMaxDistances.DEFAULT, false);
	}

	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		PlayerBase other = PlayerBase.Cast(target.GetObject());
		if (!other || other == player || !other.IsAlive())
			return false;
		// both: logged in, a working full KPK of their own in the hands (the server syncs the flag)
		if (GetGame().IsDedicatedServer())
		{
			KPK_Server server = KPK_Server.Get();
			if (!server || !player.GetIdentity() || !other.GetIdentity())
				return false;
			return server.ReadyForContact(player, player.GetIdentity().GetPlainId()) && server.ReadyForContact(other, other.GetIdentity().GetPlainId());
		}
		return player.KPK_IsReady() && other.KPK_IsReady();
	}

	override void OnExecuteServer(ActionData action_data)
	{
		super.OnExecuteServer(action_data);
		PlayerBase other = PlayerBase.Cast(action_data.m_Target.GetObject());
		if (KPK_Server.Get())
			KPK_Server.Get().OnWorldAction(action_data.m_Player, other, m_Invite);
	}
};

class ActionKPKWorldInvite : ActionKPKWorldContact
{
	void ActionKPKWorldInvite()
	{
		m_Invite = true;
		m_Text = "КПК: пригласить в группу";
	}
};
