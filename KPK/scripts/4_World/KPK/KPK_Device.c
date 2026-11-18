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

	// the broken KPK is a separate item (not a KPK_Device): it can only be switched on once
	static bool IsBroken(EntityAI item)
	{
		return item && item.IsKindOf("KPK_Broken");
	}

	// which model a device is (KPK_Model); the brick is the base class, so test the others first
	static int ModelOf(EntityAI item)
	{
		if (!item)
			return KPK_Model.BRICK;
		if (item.IsKindOf("KPK_Stalker"))
			return KPK_Model.STALKER;
		if (item.IsKindOf("KPK_Military"))
			return KPK_Model.MILITARY;
		if (item.IsKindOf("KPK_Monolith"))
			return KPK_Model.MONOLITH;
		if (item.IsKindOf("KPK_Science"))
			return KPK_Model.SCIENCE;
		return KPK_Model.BRICK;
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

// The engine runs the KPK with the GPS script (the config inherits GPSReceiver). Keep only the parts
// the KPK needs (battery slot, hands); the GPS display would overwrite the KPK texture with its digits.
modded class GPSReceiver
{
	override void SetActions()
	{
		super.SetActions();
		AddAction(ActionKPKOpen);
		AddAction(ActionKPKBroken);
	}

	// every KPK model holds three copies of its mesh (model.cfg): in the hands, on a strap and free
	// (inventory, ground, bag: centred, so the inspect view turns it around its middle)
	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);
		if (KPK_Device.IsKPK(this) || KPK_Device.IsBroken(this))
			KPK_UpdatePose();
	}

	override void EEInit()
	{
		super.EEInit();
		if (KPK_Device.IsKPK(this) || KPK_Device.IsBroken(this))
		{
			KPK_UpdatePose();
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KPK_UpdatePose, 500, false);
		}
	}

	// inventory preview: frame the copy that is shown (model memory points boundingbox_min2 / invView2 ...)
	override int GetViewIndex()
	{
		if (!(KPK_Device.IsKPK(this) || KPK_Device.IsBroken(this)) || !MemoryPointExists("invView2"))
			return super.GetViewIndex();
		InventoryLocation il = new InventoryLocation();
		if (!GetInventory().GetCurrentInventoryLocation(il))
			return 0;
		if (il.GetType() == InventoryLocationType.HANDS)
			return 1;
		if (il.GetType() == InventoryLocationType.ATTACHMENT)
			return 2;
		return 0;
	}

	void KPK_UpdatePose()
	{
		InventoryLocation loc = new InventoryLocation();
		int locType = InventoryLocationType.UNKNOWN;
		if (GetInventory().GetCurrentInventoryLocation(loc))
			locType = loc.GetType();
		float hideHand = 1;
		float hideStrap = 1;
		float hideFree = 0;
		if (locType == InventoryLocationType.HANDS)
		{
			hideHand = 0;
			hideFree = 1;
		}
		else if (locType == InventoryLocationType.ATTACHMENT)
		{
			hideStrap = 0;
			hideFree = 1;
		}
		SetAnimationPhase("kpkNotHand", hideHand);
		SetAnimationPhase("kpkNotStrap", hideStrap);
		SetAnimationPhase("kpkNotFree", hideFree);
	}

	override void OnWorkStart()
	{
		if (KPK_Device.IsKPK(this) || KPK_Device.IsBroken(this))
			return;
		super.OnWorkStart();
	}

	override void OnWorkStop()
	{
		if (KPK_Device.IsKPK(this) || KPK_Device.IsBroken(this))
			return;
		super.OnWorkStop();
	}

	override void OnWork(float consumed_energy)
	{
		if (KPK_Device.IsKPK(this) || KPK_Device.IsBroken(this))
			return;
		super.OnWork(consumed_energy);
	}
};

// the vanilla GPS on/off actions must not show up on the KPK
modded class ActionTurnOnWhileInHands
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (KPK_Device.IsKPK(item) || KPK_Device.IsBroken(item))
			return false;
		return super.ActionCondition(player, target, item);
	}
};

modded class ActionTurnOffWhileInHands
{
	override bool ActionCondition(PlayerBase player, ActionTarget target, ItemBase item)
	{
		if (KPK_Device.IsKPK(item) || KPK_Device.IsBroken(item))
			return false;
		return super.ActionCondition(player, target, item);
	}
};
