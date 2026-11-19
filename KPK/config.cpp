class CfgPatches
{
	class KPK
	{
		units[] = {"KPK_Device", "KPK_Stalker", "KPK_Military", "KPK_Monolith", "KPK_Science", "KPK_Broken"};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "DZ_Scripts", "DZ_Gear_Navigation", "DZ_Radio"};
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

	class PersonalRadio;

	// Common base of the full models (KPK-M1, ВПК-7М, Монолит, НИИ): the vanilla GPS gives the 9V battery
	// slot, energy manager and the way it is held in hands.
	class KPK_Base: GPSReceiver
	{
		scope = 0;
		// textures come from the model itself; no hiddenSelections on purpose: the inherited GPS
		// script writes its display digits into hidden selection 0
		hiddenSelections[] = {};
		hiddenSelectionsTextures[] = {};
		hiddenSelectionsMaterials[] = {};
		weight = 450;
		itemSize[] = {2, 2};
		// set by the script (pose copies in the model): 0 for where the KPK is, 1 for the rest
		class AnimationSources
		{
			class kpkNotHand
			{
				source = "user";
				initPhase = 1;
				animPeriod = 0.001;
			};
			class kpkNotStrap: kpkNotHand {};
			class kpkNotFree: kpkNotHand
			{
				initPhase = 0;
			};
		};
	};

	// The brick (own model KPK/data/kpk.p3d): profile, map, party, and a radio for calls between bricks.
	// It is a vanilla walkie-talkie underneath (9V battery, voice over the radio while a call is on).
	class KPK_Device: PersonalRadio
	{
		scope = 2;
		model = "\KPK\data\kpk.p3d";
		hiddenSelections[] = {};
		hiddenSelectionsTextures[] = {};
		hiddenSelectionsMaterials[] = {};
		displayName = "КПК сталкера";
		descriptionShort = "Базовый карманный компьютер «Вестник»: профиль, карта с метками, связь с группой и рация для вызова другого «Вестника». Работает от батарейки 9V.";
		weight = 300;
		itemSize[] = {1, 2};
		// set by the script (pose copies in the model): 0 for where the KPK is, 1 for the rest
		class AnimationSources
		{
			class kpkNotHand
			{
				source = "user";
				initPhase = 1;
				animPeriod = 0.001;
			};
			class kpkNotStrap: kpkNotHand {};
			class kpkNotFree: kpkNotHand
			{
				initPhase = 0;
			};
		};
	};

	// Full models: everything the brick has plus factions, tasks, notes and reputation,
	// each with its own interface theme (KPK_Theme in the scripts).
	class KPK_Stalker: KPK_Base
	{
		scope = 2;
		displayName = "КПК-М1 сталкера";
		descriptionShort = "Карманный персональный компьютер, модель 1. Полная версия сети сталкеров: карта, группа, группировки, задания, заметки. Батарейка 9V.";
		model = "\KPK\data\kpk_m1.p3d";
		weight = 450;
		itemSize[] = {3, 2};
	};
	class KPK_Military: KPK_Base
	{
		scope = 2;
		displayName = "ВПК-7М (ОКСОП)";
		descriptionShort = "Военный персональный компьютер Объединённого контингента сил охраны периметра. Закрытый канал связи. Батарейка 9V.";
		model = "\KPK\data\kpk_mil.p3d";
		weight = 700;
		itemSize[] = {3, 2};
	};
	class KPK_Monolith: KPK_Base
	{
		scope = 2;
		displayName = "ПИК №ОС-031";
		descriptionShort = "Персональный терминал связи с Осознанием. От экрана веет холодом. Батарейка 9V.";
		model = "\KPK\data\kpk_mono.p3d";
		weight = 500;
		itemSize[] = {3, 2};
	};
	class KPK_Science: KPK_Base
	{
		scope = 2;
		displayName = "ПИК №042 (НИИ)";
		descriptionShort = "Персональный исследовательский компьютер научно-исследовательского института. Батарейка 9V.";
		model = "\KPK\data\kpk_nii.p3d";
		weight = 550;
		itemSize[] = {3, 2};
	};

	// Broken KPK: not a working device. Switched on once with a battery it shows a random grid square,
	// then burns out (ruined, battery drained, 10 damage to the player).
	class KPK_Broken: GPSReceiver
	{
		scope = 2;
		displayName = "Разбитый КПК";
		descriptionShort = "Обгоревший КПК с треснувшим экраном. Может быть, в памяти что-то осталось. Батарейка 9V.";
		model = "\KPK\data\kpk_broken.p3d";
		hiddenSelections[] = {};
		hiddenSelectionsTextures[] = {};
		hiddenSelectionsMaterials[] = {};
		weight = 280;
		itemSize[] = {1, 2};
		class AnimationSources
		{
			class kpkNotHand
			{
				source = "user";
				initPhase = 1;
				animPeriod = 0.001;
			};
			class kpkNotStrap: kpkNotHand {};
			class kpkNotFree: kpkNotHand
			{
				initPhase = 0;
			};
		};
	};
};

// interface sounds (source/sounds/gen_sounds.py); played on the client only, at the camera
class CfgSoundShaders
{
	class KPK_click_SoundShader
	{
		samples[] = {{"KPK\data\sounds\kpk_click", 1}};
		volume = 1.0;
		range = 6;
	};
	class KPK_tab_SoundShader
	{
		samples[] = {{"KPK\data\sounds\kpk_tab", 1}};
		volume = 1.0;
		range = 6;
	};
	class KPK_beep1_SoundShader
	{
		samples[] = {{"KPK\data\sounds\kpk_beep1", 1}};
		volume = 1.0;
		range = 6;
	};
	class KPK_beep2_SoundShader
	{
		samples[] = {{"KPK\data\sounds\kpk_beep2", 1}};
		volume = 1.0;
		range = 6;
	};
	class KPK_data_SoundShader
	{
		samples[] = {{"KPK\data\sounds\kpk_data", 1}};
		volume = 1.0;
		range = 6;
	};
	class KPK_static_SoundShader
	{
		samples[] = {{"KPK\data\sounds\kpk_static", 1}};
		volume = 1.0;
		range = 6;
	};
	class KPK_whisper_SoundShader
	{
		samples[] = {{"KPK\data\sounds\kpk_whisper", 1}};
		volume = 1.0;
		range = 6;
	};
	class KPK_msg_SoundShader
	{
		samples[] = {{"KPK\data\sounds\kpk_msg", 1}};
		volume = 1.0;
		range = 6;
	};
};

class CfgSoundSets
{
	class KPK_click_SoundSet
	{
		soundShaders[] = {"KPK_click_SoundShader"};
		volumeFactor = 1.0;
		frequencyFactor = 1.0;
		spatial = 0;
		doppler = 0;
		loop = 0;
	};
	class KPK_tab_SoundSet
	{
		soundShaders[] = {"KPK_tab_SoundShader"};
		volumeFactor = 1.0;
		frequencyFactor = 1.0;
		spatial = 0;
		doppler = 0;
		loop = 0;
	};
	class KPK_beep1_SoundSet
	{
		soundShaders[] = {"KPK_beep1_SoundShader"};
		volumeFactor = 1.0;
		frequencyFactor = 1.0;
		spatial = 0;
		doppler = 0;
		loop = 0;
	};
	class KPK_beep2_SoundSet
	{
		soundShaders[] = {"KPK_beep2_SoundShader"};
		volumeFactor = 1.0;
		frequencyFactor = 1.0;
		spatial = 0;
		doppler = 0;
		loop = 0;
	};
	class KPK_data_SoundSet
	{
		soundShaders[] = {"KPK_data_SoundShader"};
		volumeFactor = 1.0;
		frequencyFactor = 1.0;
		spatial = 0;
		doppler = 0;
		loop = 0;
	};
	class KPK_static_SoundSet
	{
		soundShaders[] = {"KPK_static_SoundShader"};
		volumeFactor = 1.0;
		frequencyFactor = 1.0;
		spatial = 0;
		doppler = 0;
		loop = 0;
	};
	class KPK_whisper_SoundSet
	{
		soundShaders[] = {"KPK_whisper_SoundShader"};
		volumeFactor = 1.0;
		frequencyFactor = 1.0;
		spatial = 0;
		doppler = 0;
		loop = 0;
	};
	class KPK_msg_SoundSet
	{
		soundShaders[] = {"KPK_msg_SoundShader"};
		volumeFactor = 1.0;
		frequencyFactor = 1.0;
		spatial = 0;
		doppler = 0;
		loop = 0;
	};
};
