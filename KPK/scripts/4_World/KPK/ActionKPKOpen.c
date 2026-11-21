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

// Looking at another player: add them to the KPK contacts / invite them to the party (their KPK must be on)
class ActionKPKWorldContact : ActionInteractBase
{
	protected bool m_Invite;

	void ActionKPKWorldContact()
	{
		m_CommandUID = DayZPlayerConstants.CMD_ACTIONMOD_INTERACTONCE;
		m_StanceMask = DayZPlayerConstants.STANCEMASK_ERECT | DayZPlayerConstants.STANCEMASK_CROUCH;
		m_Text = "КПК: добавить в контакты";
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
		if (!m_Invite && ContactNeedsHands())
			return HoldsKPK(player) && HoldsKPK(other);
		return KPK_Device.FindOn(player) != null;
	}

	protected bool ContactNeedsHands()
	{
		if (GetGame().IsDedicatedServer())
			return !KPK_Server.Get() || KPK_Server.Get().m_Settings.ContactNeedsHands;
		return KPK_Client.s_ContactNeedsHands;
	}

	// a KPK (not ruined) in the hands; the other player's hands item is synced to every client
	protected bool HoldsKPK(PlayerBase p)
	{
		EntityAI item = p.GetHumanInventory().GetEntityInHands();
		return KPK_Device.IsKPK(item) && !item.IsRuined();
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
