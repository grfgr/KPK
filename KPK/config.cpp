class CfgPatches
{
	class KPK
	{
		units[] = {"KPK_Device"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Navigation"};
	};
};

class CfgMods
{
	class KPK
	{
		dir = "KPK";
		name = "KPK";
		credits = "";
		author = "";
		authorID = "0";
		version = "0.1.0";
		extra = 0;
		type = "mod";
		hideName = 1;
		hidePicture = 1;
		picture = "";
		action = "";
		inputs = "KPK/data/inputs.xml";
		dependencies[] = {"Game", "World", "Mission"};

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"KPK/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"KPK/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"KPK/scripts/5_Mission"};
			};
		};
	};
};

class CfgVehicles
{
	class GPSReceiver;

	// Own model (KPK/data/kpk.p3d). The class still inherits the vanilla GPS for its 9V battery slot,
	// energy manager and the way it is held in hands.
	class KPK_Device: GPSReceiver
	{
		scope = 2;
		model = "\KPK\data\kpk.p3d";
		hiddenSelections[] = {"camo"};
		hiddenSelectionsTextures[] = {"\KPK\data\kpk_co.paa"};
		hiddenSelectionsMaterials[] = {"\KPK\data\kpk.rvmat"};
		displayName = "КПК сталкера";
		descriptionShort = "Карманный персональный компьютер. Сеть Зоны, карта, связь с группой и группировкой, личные заметки. Работает от батарейки 9V.";
		weight = 300;
		itemSize[] = {1, 2};
	};
};
