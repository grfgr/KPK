class KPK_Device extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionKPKOpen);
	}

	Battery9V GetBattery()
	{
		int count = GetInventory().AttachmentCount();
		for (int i = 0; i < count; i++)
		{
			Battery9V battery = Battery9V.Cast(GetInventory().GetAttachmentFromIndex(i));
			if (battery)
				return battery;
		}
		return null;
	}

	// 0..1, -1 = no battery
	float GetBatteryLevel()
	{
		Battery9V battery = GetBattery();
		if (!battery || !battery.HasEnergyManager())
			return -1;

		float maxEnergy = battery.GetCompEM().GetEnergyMax();
		if (maxEnergy <= 0)
			return 0;
		return battery.GetCompEM().GetEnergy() / maxEnergy;
	}

	// server: drain a share of the battery capacity, returns the level left
	float Drain(float percent)
	{
		Battery9V battery = GetBattery();
		if (!battery || !battery.HasEnergyManager())
			return -1;

		float amount = battery.GetCompEM().GetEnergyMax() * percent / 100.0;
		if (amount > 0)
			battery.GetCompEM().AddEnergy(-amount);
		return GetBatteryLevel();
	}

	// hands first, then the whole inventory; with needPower the first device with a charged battery wins
	static KPK_Device FindOn(PlayerBase player, bool needPower = false)
	{
		if (!player)
			return null;

		KPK_Device best = null;
		KPK_Device inHands = KPK_Device.Cast(player.GetHumanInventory().GetEntityInHands());
		if (inHands)
		{
			if (!needPower || inHands.GetBatteryLevel() > 0)
				return inHands;
			best = inHands;
		}

		array<EntityAI> items = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI item : items)
		{
			KPK_Device device = KPK_Device.Cast(item);
			if (!device || device.IsRuined())
				continue;
			if (!needPower || device.GetBatteryLevel() > 0)
				return device;
			if (!best)
				best = device;
		}
		return best;
	}
};
