// The KPK config class inherits the vanilla GPS. Depending on how the engine resolves the script class,
// the item may run as KPK_Device or as GPSReceiver, so everything here works on any EntityAI and
// recognises the device by its config class (IsKindOf), never by a script cast.
class KPK_Device extends ItemBase
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionKPKOpen);
	}

	static bool IsKPK(EntityAI item)
	{
		return item && item.IsKindOf("KPK_Device");
	}

	static Battery9V GetBattery(EntityAI device)
	{
		if (!device || !device.GetInventory())
			return null;
		int count = device.GetInventory().AttachmentCount();
		for (int i = 0; i < count; i++)
		{
			Battery9V battery = Battery9V.Cast(device.GetInventory().GetAttachmentFromIndex(i));
			if (battery)
				return battery;
		}
		return null;
	}

	// 0..1, -1 = no battery
	static float GetBatteryLevel(EntityAI device)
	{
		Battery9V battery = GetBattery(device);
		if (!battery || !battery.HasEnergyManager())
			return -1;

		float maxEnergy = battery.GetCompEM().GetEnergyMax();
		if (maxEnergy <= 0)
			return 0;
		return battery.GetCompEM().GetEnergy() / maxEnergy;
	}

	// server: drain a share of the battery capacity, returns the level left
	static float Drain(EntityAI device, float percent)
	{
		Battery9V battery = GetBattery(device);
		if (!battery || !battery.HasEnergyManager())
			return -1;

		float amount = battery.GetCompEM().GetEnergyMax() * percent / 100.0;
		if (amount > 0)
			battery.GetCompEM().AddEnergy(-amount);
		return GetBatteryLevel(device);
	}

	// hands first, then the whole inventory; with needPower the first device with a charged battery wins
	static EntityAI FindOn(PlayerBase player, bool needPower = false)
	{
		if (!player)
			return null;

		EntityAI best = null;
		EntityAI inHands = player.GetHumanInventory().GetEntityInHands();
		if (IsKPK(inHands))
		{
			if (!needPower || GetBatteryLevel(inHands) > 0)
				return inHands;
			best = inHands;
		}

		array<EntityAI> items = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		foreach (EntityAI item : items)
		{
			if (!IsKPK(item) || item.IsRuined())
				continue;
			if (!needPower || GetBatteryLevel(item) > 0)
				return item;
			if (!best)
				best = item;
		}
		return best;
	}
};

// in case the engine runs the KPK with the GPS script: the action checks IsKPK, plain GPS units never show it
modded class GPSReceiver
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionKPKOpen);
	}
};
