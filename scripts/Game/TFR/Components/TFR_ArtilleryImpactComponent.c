//------------------------------------------------------------------------------------------------
// TFR_ArtilleryImpactComponent.c
// TFR OPERATIONS
//
// Prefab runtime spawneable para impacto de artilleria/CAS.
// Se coloca en TFR_ArtilleryImpact.et.
//
// - Al iniciar, puede spawnear un prefab visual opcional.
// - Se borra solo tras unos segundos.
// - Sirve como wrapper seguro para que EnemyComms no intente cargar ammo/effect modules directamente.
//
// SERVER POLISH:
// - Debug apagado por defecto.
// - No cambia la lógica del impacto.
// - No cambia CallLater.
// - No cambia el spawn visual.
// - Los errores críticos siguen visibles.
//------------------------------------------------------------------------------------------------

class TFR_ArtilleryImpactComponentClass : ScriptComponentClass
{
}

class TFR_ArtilleryImpactComponent : ScriptComponent
{
	[Attribute("1", UIWidgets.CheckBox, "Activar impacto")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebug;

	[Attribute("0.05", UIWidgets.EditBox, "Delay antes de ejecutar impacto")]
	protected float m_fImpactDelaySeconds;

	[Attribute("5.0", UIWidgets.EditBox, "Vida del prefab antes de borrarse")]
	protected float m_fLifeTimeSeconds;

	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab visual opcional al impactar", "et")]
	protected ResourceName m_sVisualImpactPrefab;

	protected IEntity m_Owner;
	protected Resource m_VisualImpactResource;
	protected bool m_bImpactExecuted;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_Owner = owner;
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_Owner = owner;

		if (!m_bEnabled)
			return;

		if (!GetGame())
			return;

		if (m_fImpactDelaySeconds < 0)
			m_fImpactDelaySeconds = 0;

		if (m_fLifeTimeSeconds <= 0)
			m_fLifeTimeSeconds = 5;

		int impactDelayMs = Math.Round(m_fImpactDelaySeconds * 1000.0);
		int deleteDelayMs = Math.Round(m_fLifeTimeSeconds * 1000.0);

		GetGame().GetCallqueue().CallLater(TFR_ExecuteImpact, impactDelayMs, false);
		GetGame().GetCallqueue().CallLater(TFR_DeleteSelf, deleteDelayMs, false);

		TFR_Log("Impact prefab inicializado en " + owner.GetOrigin().ToString());
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(TFR_ExecuteImpact);
			GetGame().GetCallqueue().Remove(TFR_DeleteSelf);
		}

		m_VisualImpactResource = null;
		m_Owner = null;

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_ExecuteImpact()
	{
		if (m_bImpactExecuted)
			return;

		m_bImpactExecuted = true;

		if (!m_Owner)
			return;

		vector pos = m_Owner.GetOrigin();

		if (GetGame() && GetGame().GetWorld())
			pos[1] = GetGame().GetWorld().GetSurfaceY(pos[0], pos[2]);

		TFR_Log("IMPACTO ARTILLERIA WRAPPER en " + pos.ToString());

		TFR_SpawnVisualImpact(pos);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_SpawnVisualImpact(vector pos)
	{
		if (m_sVisualImpactPrefab == "")
		{
			TFR_Log("Sin prefab visual opcional. Wrapper ejecutado correctamente.");
			return;
		}

		if (!m_VisualImpactResource || !m_VisualImpactResource.IsValid())
		{
			m_VisualImpactResource = Resource.Load(m_sVisualImpactPrefab);

			if (!m_VisualImpactResource || !m_VisualImpactResource.IsValid())
			{
				Print("[TFR_ARTY_IMPACT] ERROR cargando prefab visual: " + m_sVisualImpactPrefab, LogLevel.ERROR);
				return;
			}
		}

		BaseWorld world = GetGame().GetWorld();

		if (!world)
			return;

		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		mat[3] = pos;
		params.Transform = mat;

		IEntity spawned = GetGame().SpawnEntityPrefab(m_VisualImpactResource, world, params);

		if (!spawned)
		{
			Print("[TFR_ARTY_IMPACT] ERROR spawneando prefab visual en " + pos.ToString(), LogLevel.ERROR);
			return;
		}

		TFR_Log("Prefab visual spawneado: " + spawned);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_DeleteSelf()
	{
		if (!m_Owner)
			return;

		if (m_Owner.IsDeleted())
			return;

		TFR_Log("Borrando wrapper de impacto.");

		SCR_EntityHelper.DeleteEntityAndChildren(m_Owner);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_Log(string msg)
	{
		if (!m_bDebug)
			return;

		Print("[TFR_ARTY_IMPACT] " + msg, LogLevel.NORMAL);
	}
}
