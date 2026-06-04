//------------------------------------------------------------------------------------------------
// TFR_SpawnAreaComponent.c
// TFR Operations
//
// Marca áreas del mundo para que Spawn Director y Mission Manager puedan usarlas.
// El nombre de la entidad NO importa.
//
// Lo importante es:
// - Area Type
// - Radius
// - Enabled
//
// SERVER POLISH:
// - Sin ticks.
// - Sin escaneos continuos.
// - Debug apagado por defecto.
// - Evita doble registro real entre OnPostInit/EOnInit.
// - RegisterArea no limpia todo el registro cada vez.
// - CleanupRegistry solo se usa en consultas.
//------------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------------
enum TFR_AreaType
{
	ANY = 0,
	TOWN,
	MILITARY,
	INDUSTRIAL,
	ROAD,
	CHECKPOINT,
	BASE,
	COMPOUND
};

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "TFR Operations/Spawn", description: "TFR spawn area marker")]
class TFR_SpawnAreaComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class TFR_SpawnAreaComponent : ScriptComponent
{
	protected static ref array<TFR_SpawnAreaComponent> s_aRegisteredAreas;

	[Attribute("1", UIWidgets.ComboBox, "Area Type", "", ParamEnumArray.FromEnum(TFR_AreaType))]
	protected TFR_AreaType m_eAreaType;

	[Attribute("", UIWidgets.EditBox, "Optional Area Name")]
	protected string m_sAreaName;

	[Attribute("75", UIWidgets.Slider, "Spawn Radius", "0 1000 1")]
	protected float m_fRadius;

	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	protected bool m_bRegistered;

	//--------------------------------------------------------------------------------------------
	void TFR_SpawnAreaComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		EnsureRegistry();
		m_bRegistered = false;
	}

	//--------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		RegisterSelfOnce();
		LogArea("EOnInit -> registered");
	}

	//--------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		RegisterSelfOnce();
		LogArea("OnPostInit -> registered");
	}

	//--------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		UnregisterArea(this);
		LogArea("OnDelete -> unregistered");

		super.OnDelete(owner);
	}

	//--------------------------------------------------------------------------------------------
	protected void RegisterSelfOnce()
	{
		if (m_bRegistered)
			return;

		RegisterArea(this);
	}

	//--------------------------------------------------------------------------------------------
	protected void LogArea(string msg)
	{
		if (!m_bDebugLogs)
			return;

		string ownerName = "<null>";
		IEntity owner = GetOwner();

		if (owner)
			ownerName = owner.GetName();

		Print("[TFR SpawnArea] " + msg + " | owner=" + ownerName + " | type=" + GetAreaTypeName() + " | enabled=" + m_bEnabled.ToString(), LogLevel.NORMAL);
	}

	//--------------------------------------------------------------------------------------------
	protected static void EnsureRegistry()
	{
		if (!s_aRegisteredAreas)
			s_aRegisteredAreas = new array<TFR_SpawnAreaComponent>();
	}

	//--------------------------------------------------------------------------------------------
	protected static void CleanupRegistry()
	{
		EnsureRegistry();

		for (int i = s_aRegisteredAreas.Count() - 1; i >= 0; i--)
		{
			TFR_SpawnAreaComponent area = s_aRegisteredAreas[i];

			if (!area)
			{
				s_aRegisteredAreas.Remove(i);
				continue;
			}

			IEntity owner = area.GetOwner();

			if (!owner || owner.IsDeleted())
			{
				area.m_bRegistered = false;
				s_aRegisteredAreas.Remove(i);
				continue;
			}
		}
	}

	//--------------------------------------------------------------------------------------------
	static void RegisterArea(TFR_SpawnAreaComponent area)
	{
		if (!area)
			return;

		EnsureRegistry();

		IEntity owner = area.GetOwner();

		if (!owner || owner.IsDeleted())
			return;

		if (s_aRegisteredAreas.Find(area) < 0)
			s_aRegisteredAreas.Insert(area);

		area.m_bRegistered = true;
	}

	//--------------------------------------------------------------------------------------------
	static void UnregisterArea(TFR_SpawnAreaComponent area)
	{
		if (!area)
			return;

		EnsureRegistry();

		int idx = s_aRegisteredAreas.Find(area);

		if (idx >= 0)
			s_aRegisteredAreas.Remove(idx);

		area.m_bRegistered = false;
	}

	//--------------------------------------------------------------------------------------------
	static void CollectAreas(TFR_AreaType requestedType, array<TFR_SpawnAreaComponent> outAreas)
	{
		if (!outAreas)
			return;

		outAreas.Clear();

		EnsureRegistry();
		CleanupRegistry();

		for (int i = 0; i < s_aRegisteredAreas.Count(); i++)
		{
			TFR_SpawnAreaComponent area = s_aRegisteredAreas[i];

			if (!area)
				continue;

			if (!area.Matches(requestedType, string.Empty))
				continue;

			if (outAreas.Find(area) < 0)
				outAreas.Insert(area);
		}
	}

	//--------------------------------------------------------------------------------------------
	static int GetRegisteredAreaCount()
	{
		EnsureRegistry();
		CleanupRegistry();

		return s_aRegisteredAreas.Count();
	}

	//--------------------------------------------------------------------------------------------
	static int GetRegisteredAreaCountByType(TFR_AreaType requestedType)
	{
		EnsureRegistry();
		CleanupRegistry();

		int count = 0;

		for (int i = 0; i < s_aRegisteredAreas.Count(); i++)
		{
			TFR_SpawnAreaComponent area = s_aRegisteredAreas[i];

			if (!area)
				continue;

			if (!area.Matches(requestedType, string.Empty))
				continue;

			count++;
		}

		return count;
	}

	//--------------------------------------------------------------------------------------------
	TFR_AreaType GetAreaType()
	{
		return m_eAreaType;
	}

	//--------------------------------------------------------------------------------------------
	string GetAreaTypeName()
	{
		switch (m_eAreaType)
		{
			case TFR_AreaType.ANY:
				return "ANY";

			case TFR_AreaType.TOWN:
				return "TOWN";

			case TFR_AreaType.MILITARY:
				return "MILITARY";

			case TFR_AreaType.INDUSTRIAL:
				return "INDUSTRIAL";

			case TFR_AreaType.ROAD:
				return "ROAD";

			case TFR_AreaType.CHECKPOINT:
				return "CHECKPOINT";

			case TFR_AreaType.BASE:
				return "BASE";

			case TFR_AreaType.COMPOUND:
				return "COMPOUND";
		}

		return "UNKNOWN";
	}

	//--------------------------------------------------------------------------------------------
	float GetRadius()
	{
		if (m_fRadius <= 0)
			return 25;

		return m_fRadius;
	}

	//--------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bEnabled;
	}

	//--------------------------------------------------------------------------------------------
	string GetAreaName()
	{
		if (m_sAreaName != string.Empty)
			return m_sAreaName;

		IEntity owner = GetOwner();

		if (!owner)
			return string.Empty;

		return owner.GetName();
	}

	//--------------------------------------------------------------------------------------------
	vector GetAreaOrigin()
	{
		IEntity owner = GetOwner();

		if (!owner)
			return vector.Zero;

		return owner.GetOrigin();
	}

	//--------------------------------------------------------------------------------------------
	bool Matches(TFR_AreaType requestedType, string requestedName)
	{
		if (!m_bEnabled)
			return false;

		if (requestedName != string.Empty)
		{
			string areaName = GetAreaName();

			if (areaName != requestedName)
				return false;
		}

		if (requestedType == TFR_AreaType.ANY)
			return true;

		return m_eAreaType == requestedType;
	}
}
