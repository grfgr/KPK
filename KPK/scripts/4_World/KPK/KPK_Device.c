// The brick (config KPK_Device) is a vanilla walkie-talkie underneath: calls between bricks use its radio.
// The full models (config KPK_Base -> GPSReceiver) run with the GPS script. Everything static here works on
// any EntityAI and recognises the devices by config class (IsKindOf), never by a script cast.
class KPK_Device extends PersonalRadio
{
	override void SetActions()
	{
		super.SetActions();
		// the radio is driven by the KPK calls only: no manual on / off / tuning
		RemoveAction(ActionTurnOnTransmitter);
		RemoveAction(ActionTurnOffTransmitter);
		RemoveAction(ActionTuneFrequency);
		AddAction(ActionKPKOpen);
	}

	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);
		UpdatePose(this);
	}

	override void EEInit()
	{
		super.EEInit();
		UpdatePose(this);
		GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KPK_PoseLater, 500, false);
	}

	void KPK_PoseLater()
	{
		UpdatePose(this);
	}

	override int GetViewIndex()
	{
		return ViewIndex(this);
	}

	// any KPK model (not the broken one)
	static bool IsKPK(EntityAI item)
	{
		return item && (item.IsKindOf("KPK_Device") || item.IsKindOf("KPK_Base"));
	}

	// the client side of a brick call: every brick on this player tuned and transmitting / receiving
	static void RadioClient(PlayerBase player, bool on, int channel)
	{
		array<EntityAI> items = new array<EntityAI>();
		player.GetInventory().EnumerateInventory(InventoryTraversalType.PREORDER, items);
		EntityAI inHands = player.GetHumanInventory().GetEntityInHands();
		if (inHands)
			items.Insert(inHands);
		foreach (EntityAI it : items)
		{
			TransmitterBase radio = TransmitterBase.Cast(it);
			if (!radio || !KPK_Device.IsBrick(it))
				continue;
			if (on)
				radio.SetFrequencyByIndex(channel);
			radio.EnableBroadcast(on);
			radio.EnableReceive(on);
		}
		int shownChannel = channel + 1;
		if (on)
			NotificationSystem.AddNotificationExtended(5, "КПК: связь", "Канал " + shownChannel.ToString() + ". Возьмите «Вестник» в руки и говорите.", "set:dayz_gui image:icon");
	}

	static bool IsBrick(EntityAI item)
	{
		return item && item.IsKindOf("KPK_Device");
	}

	// every KPK model holds three copies of its mesh (model.cfg): in the hands, on a strap and free
	// (inventory, ground, bag: centred, so the inspect view turns it around its middle)
	static void UpdatePose(EntityAI item)
	{
		if (!item)
			return;
		InventoryLocation loc = new InventoryLocation();
		int locType = InventoryLocationType.UNKNOWN;
		if (item.GetInventory().GetCurrentInventoryLocation(loc))
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
		item.SetAnimationPhase("kpkNotHand", hideHand);
		item.SetAnimationPhase("kpkNotStrap", hideStrap);
		item.SetAnimationPhase("kpkNotFree", hideFree);
	}

	// inventory preview: frame the copy that is shown (model memory points boundingbox_min2 / invView2 ...)
	static int ViewIndex(EntityAI item)
	{
		if (!item.MemoryPointExists("invView2"))
			return 0;
		InventoryLocation il = new InventoryLocation();
		if (!item.GetInventory().GetCurrentInventoryLocation(il))
			return 0;
		if (il.GetType() == InventoryLocationType.HANDS)
			return 1;
		if (il.GetType() == InventoryLocationType.ATTACHMENT)
			return 2;
		return 0;
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

	override void EEItemLocationChanged(notnull InventoryLocation oldLoc, notnull InventoryLocation newLoc)
	{
		super.EEItemLocationChanged(oldLoc, newLoc);
		if (KPK_Device.IsKPK(this) || KPK_Device.IsBroken(this))
			KPK_Device.UpdatePose(this);
	}

	override void EEInit()
	{
		super.EEInit();
		if (KPK_Device.IsKPK(this) || KPK_Device.IsBroken(this))
		{
			KPK_Device.UpdatePose(this);
			GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(KPK_PoseLater, 500, false);
		}
	}

	void KPK_PoseLater()
	{
		KPK_Device.UpdatePose(this);
	}

	override int GetViewIndex()
	{
		if (KPK_Device.IsKPK(this) || KPK_Device.IsBroken(this))
			return KPK_Device.ViewIndex(this);
		return super.GetViewIndex();
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

// the brick is a walkie-talkie underneath, but is held like the GPS (the pose the model was made for)
modded class ModItemRegisterCallbacks
{
	override void RegisterOneHanded(DayZPlayerType pType, DayzPlayerItemBehaviorCfg pBehavior)
	{
		super.RegisterOneHanded(pType, pBehavior);
		pType.AddItemInHandsProfileIK("KPK_Device", "dz/anims/workspaces/player/player_main/props/player_main_1h_GPSReciever.asi", pBehavior, "dz/anims/anm/player/ik/gear/GPSReciever.anm");
	}
};
