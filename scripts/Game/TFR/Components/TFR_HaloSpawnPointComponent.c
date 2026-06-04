//------------------------------------------------------------------------------------------------
// TFR_HaloSpawnPointComponent.c
// TFR OPERATIONS
//
// Punto físico de inserción HALO.
//
// Uso:
// - Crear un prefab/entidad simple en el mapa.
// - Añadir este componente.
// - Colocarlo donde quieras que sea el centro de la DZ HALO.
// - El TFR_HaloJumpComponent lo usará antes que el GameMode.
//
// No tiene loops.
// No spawnea nada.
// Solo registra puntos activos.
//------------------------------------------------------------------------------------------------

class TFR_HaloSpawnPointComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class TFR_HaloSpawnPointComponent : ScriptComponent
{
	protected static ref array<TFR_HaloSpawnPointComponent> s_aSpawnPoints;

	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute("HALO DZ", UIWidgets.EditBox, "Spawn Point Name")]
	protected string m_sSpawnPointName;

	[Attribute("0", UIWidgets.EditBox, "Priority. Higher value wins.")]
	protected int m_iPriority;

	protected bool m_bRegistered;

	//------------------------------------------------------------------------------------------------
	void TFR_HaloSpawnPointComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		EnsureRegistry();

		m_bEnabled = true;
		m_bDebugLogs = false;
		m_sSpawnPointName = "HALO DZ";
		m_iPriority = 0;
		m_bRegistered = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		RegisterSelfOnce();
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		RegisterSelfOnce();
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		UnregisterSpawnPoint(this);

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterSelfOnce()
	{
		if (m_bRegistered)
			return;

		RegisterSpawnPoint(this);
		LogTFR("Registered spawn point: " + GetSpawnPointName());
	}

	//------------------------------------------------------------------------------------------------
	protected static void EnsureRegistry()
	{
		if (!s_aSpawnPoints)
			s_aSpawnPoints = new array<TFR_HaloSpawnPointComponent>();
	}

	//------------------------------------------------------------------------------------------------
	protected static void CleanupRegistry()
	{
		EnsureRegistry();

		for (int i = s_aSpawnPoints.Count() - 1; i >= 0; i--)
		{
			TFR_HaloSpawnPointComponent point = s_aSpawnPoints[i];

			if (!point)
			{
				s_aSpawnPoints.Remove(i);
				continue;
			}

			IEntity owner = point.GetOwner();

			if (!owner || owner.IsDeleted())
			{
				point.m_bRegistered = false;
				s_aSpawnPoints.Remove(i);
				continue;
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	static void RegisterSpawnPoint(TFR_HaloSpawnPointComponent point)
	{
		if (!point)
			return;

		EnsureRegistry();

		IEntity owner = point.GetOwner();

		if (!owner || owner.IsDeleted())
			return;

		if (s_aSpawnPoints.Find(point) < 0)
			s_aSpawnPoints.Insert(point);

		point.m_bRegistered = true;
	}

	//------------------------------------------------------------------------------------------------
	static void UnregisterSpawnPoint(TFR_HaloSpawnPointComponent point)
	{
		if (!point)
			return;

		EnsureRegistry();

		int idx = s_aSpawnPoints.Find(point);

		if (idx >= 0)
			s_aSpawnPoints.Remove(idx);

		point.m_bRegistered = false;
	}

	//------------------------------------------------------------------------------------------------
	static TFR_HaloSpawnPointComponent GetBestSpawnPoint()
	{
		EnsureRegistry();
		CleanupRegistry();

		TFR_HaloSpawnPointComponent bestPoint = null;
		int bestPriority = -999999;

		foreach (TFR_HaloSpawnPointComponent point : s_aSpawnPoints)
		{
			if (!point)
				continue;

			if (!point.IsEnabled())
				continue;

			IEntity owner = point.GetOwner();

			if (!owner || owner.IsDeleted())
				continue;

			if (!bestPoint)
			{
				bestPoint = point;
				bestPriority = point.GetPriority();
				continue;
			}

			if (point.GetPriority() > bestPriority)
			{
				bestPoint = point;
				bestPriority = point.GetPriority();
			}
		}

		return bestPoint;
	}

	//------------------------------------------------------------------------------------------------
	static bool GetBestSpawnPosition(out vector outPos)
	{
		outPos = vector.Zero;

		TFR_HaloSpawnPointComponent point = GetBestSpawnPoint();

		if (!point)
			return false;

		outPos = point.GetOrigin();
		return outPos != vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	bool IsEnabled()
	{
		return m_bEnabled;
	}

	//------------------------------------------------------------------------------------------------
	int GetPriority()
	{
		return m_iPriority;
	}

	//------------------------------------------------------------------------------------------------
	string GetSpawnPointName()
	{
		if (m_sSpawnPointName != string.Empty)
			return m_sSpawnPointName;

		IEntity owner = GetOwner();

		if (owner)
			return owner.GetName();

		return "HALO DZ";
	}

	//------------------------------------------------------------------------------------------------
	vector GetOrigin()
	{
		IEntity owner = GetOwner();

		if (!owner)
			return vector.Zero;

		return owner.GetOrigin();
	}

	//------------------------------------------------------------------------------------------------
	protected void LogTFR(string msg)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR HALO SPAWNPOINT] " + msg, LogLevel.NORMAL);
	}
}
