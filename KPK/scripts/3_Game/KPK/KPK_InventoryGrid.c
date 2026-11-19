// The KPK models hold three copies of the mesh (hands / strap / free) and frame the visible one through
// GetViewIndex. The quickbar sets the view only when an item is added to it, so after the device was moved
// the slot kept framing the old copy (a grey square: a close-up of the case). Re-apply the view on every
// quickbar refresh.
modded class InventoryGrid
{
	override void RefreshItemVariables(InventoryItem item, vector data, bool show_quantity, bool show_temperature)
	{
		super.RefreshItemVariables(item, data, show_quantity, show_temperature);
		if (!item || !(item.IsKindOf("KPK_Device") || item.IsKindOf("KPK_Base") || item.IsKindOf("KPK_Broken")))
			return;
		int index = Math.Round(data[0]);
		Widget bck = GetItemBackground(index);
		if (!bck)
			return;
		ItemPreviewWidget preview = ItemPreviewWidget.Cast(bck.FindAnyWidget("Preview"));
		if (preview && preview.GetItem() == item)
			preview.SetView(item.GetViewIndex());
	}
};
