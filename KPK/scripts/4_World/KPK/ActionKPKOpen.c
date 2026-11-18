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
	}
};
