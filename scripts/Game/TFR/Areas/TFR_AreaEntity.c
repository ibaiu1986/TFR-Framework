//------------------------------------------------------------------------------------------------
// TFR_AreaEntity.c
// TFR FRAMEWORK / TFR OPERATIONS
//
// Scripted TFR area entity.
//
// GOAL:
// Mission maker places a ready prefab such as:
// - TFR_Area_Town.et
// - TFR_Area_Road.et
// - TFR_Area_Military.et
// - TFR_Area_Compound.et
//
// Mission maker only configures a few important fields:
// - Area Type
// - Radius
// - Enabled
// - Rule Preset
// - Debug Logs
//
// Internal script handles:
// - area registration
// - simple runtime validation
// - scan data container
// - future Environment Scanner / SpawnDirector access
//
// No loose GenericEntities with magic names.
// No component stacking required from mission makers.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
enum TFR_EAreaRulePreset
{
	NONE = 0,
	AMBIENT_CIVILIAN = 1,
	AMBIENT_TRAFFIC = 2,
	VILLAGE_LIFE = 3,
	MILITARY_PATROL = 4,
	CHECKPOINT = 5,
	FULL_AREA_LIFE = 6
}

//------------------------------------------------------------------------------------------------
[EntityEditorProps(
	category: "TFR Operations/Areas",
	description: "TFR scripted area entity. Place this instead of loose GenericEntities.",
	color: "0 180 255 255",
	visible: true,
	insertable: true,
	style: "box",
	sizeMin: "-0.5 0 -0.5",
	sizeMax: "0.5 1 0.5"
)]
class TFR_AreaEntityClass : GenericEntityClass
{
}

//------------------------------------------------------------------------------------------------
class TFR_AreaEntity : GenericEntity
{
	protected static ref array<TFR_AreaEntity> s_aRegisteredAreas;

	[Attribute("true", UIWidgets.CheckBox, "Enabled", category: "TFR Area")]
	protected bool m_bEnabled;

	[Attribute("1", UIWidgets.ComboBox, "Area Type", "", ParamEnumArray.FromEnum(TFR_AreaType), category: "TFR Area")]
	protected TFR_AreaType m_eAreaType;

	[Attribute("6", UIWidgets.ComboBox, "Rule Preset", "", ParamEnumArray.FromEnum(TFR_EAreaRulePreset), category: "TFR Area")]
	protected TFR_EAreaRulePreset m_eRulePreset;

	[Attribute("150", UIWidgets.Slider, "Radius", "25 1000 1", category: "TFR Area")]
	protected float m_fRadius;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs", category: "TFR Area")]
	protected bool m_bDebugLogs;

	protected bool m_bRegistered;
	protected ref TFR_AreaEnvironmentData m_EnvironmentData;

	//------------------------------------------------------------------------------------------------
	void TFR_AreaEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);

		EnsureRegistry();

		m_bEnabled = true;
		m_eAreaType = TFR_AreaType.TOWN;
		m_eRulePreset = TFR_EAreaRulePreset.FULL_AREA_LIFE;
		m_fRadius = 150;
		m_bDebugLogs = false;
		m_bRegistered = false;

		m_EnvironmentData = new TFR_AreaEnvironmentData();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		RegisterSelfOnce();
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		UnregisterArea(this);

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterSelfOnce()
	{
		if (m_bRegistered)
			return;

		RegisterArea(this);
		LogArea("Registered TFR scripted area.");
	}

	//------------------------------------------------------------------------------------------------
	protected static void EnsureRegistry()
	{
		if (!s_aRegisteredAreas)
			s_aRegisteredAreas = new array<TFR_AreaEntity>();
	}

	//------------------------------------------------------------------------------------------------
	protected static void CleanupRegistry()
	{
		EnsureRegistry();

		for (int i = s_aRegisteredAreas.Count() - 1; i >= 0; i--)
		{
			TFR_AreaEntity area = s_aRegisteredAreas[i];

			if (!area)
			{
				s_aRegisteredAreas.Remove(i);
				continue;
			}

			if (area.IsDeleted())
			{
				area.m_bRegistered = false;
				s_aRegisteredAreas.Remove(i);
				continue;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	static void RegisterArea(TFR_AreaEntity area)
	{
		if (!area)
			return;

		EnsureRegistry();

		if (area.IsDeleted())
			return;

		if (s_aRegisteredAreas.Find(area) < 0)
			s_aRegisteredAreas.Insert(area);

		area.m_bRegistered = true;
	}

	//------------------------------------------------------------------------------------------------
	static void UnregisterArea(TFR_AreaEntity area)
	{
		if (!area)
			return;

		EnsureRegistry();

		int idx = s_aRegisteredAreas.Find(area);

		if (idx >= 0)
			s_aRegisteredAreas.Remove(idx);

		area.m_bRegistered = false;
	}

	//------------------------------------------------------------------------------------------------
	static void CollectAreas(TFR_AreaType requestedType, array<TFR_AreaEntity> outAreas)
	{
		if (!outAreas)
			return;

		outAreas.Clear();

		EnsureRegistry();
		CleanupRegistry();

		foreach (TFR_AreaEntity area : s_aRegisteredAreas)
		{
			if (!area)
				continue;

			if (!area.IsEnabled())
				continue;

			if (requestedType != TFR_AreaType.ANY && area.GetAreaType() != requestedType)
				continue;

			outAreas.Insert(area);
		}
	}

	//------------------------------------------------------------------------------------------------
	static int GetRegisteredAreaCount()
	{
		EnsureRegistry();
		CleanupRegistry();

		return s_aRegisteredAreas.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	TFR_AreaType GetAreaType()
	{
		return m_eAreaType;
	}

	//------------------------------------------------------------------------------------------------
	TFR_EAreaRulePreset GetRulePreset()
	{
		return m_eRulePreset;
	}

	//------------------------------------------------------------------------------------------------
	float GetRadius()
	{
		if (m_fRadius <= 0)
			return 25;

		return m_fRadius;
	}

	//------------------------------------------------------------------------------------------------
	vector GetAreaOrigin()
	{
		return GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	TFR_AreaEnvironmentData GetEnvironmentData()
	{
		if (!m_EnvironmentData)
			m_EnvironmentData = new TFR_AreaEnvironmentData();

		return m_EnvironmentData;
	}

	//------------------------------------------------------------------------------------------------
	bool HasRulePreset(TFR_EAreaRulePreset preset)
	{
		if (m_eRulePreset == TFR_EAreaRulePreset.FULL_AREA_LIFE)
			return true;

		return m_eRulePreset == preset;
	}

	//------------------------------------------------------------------------------------------------
	string GetDebugName()
	{
		string name = GetName();

		if (name != string.Empty)
			return name;

		return "TFR_AreaEntity";
	}

	//------------------------------------------------------------------------------------------------
	override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
	{
		float radius = m_fRadius;

		if (radius <= 0)
			radius = 25;

		min = Vector(-radius, 0, -radius);
		max = Vector(radius, 1, radius);
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_ShouldShowBoundBox(IEntitySource src)
	{
		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void LogArea(string message)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR AREA ENTITY] " + message + " Name=" + GetDebugName() + " Type=" + m_eAreaType.ToString() + " Preset=" + m_eRulePreset.ToString() + " Radius=" + GetRadius().ToString(), LogLevel.NORMAL);
	}
}
