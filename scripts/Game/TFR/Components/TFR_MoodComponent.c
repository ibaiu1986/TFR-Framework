class TFR_MoodComponentClass : ScriptComponentClass
{
}

enum TFR_EMoodState
{
	CALM = 0,
	NERVOUS = 1,
	SCARED = 2,
	AGGRESSIVE = 3
}

enum TFR_EMoodPersonality
{
	REACTIVE = 0,
	CALM = 1
}

class TFR_MoodComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.ComboBox, "Civil Personality", "", ParamEnumArray.FromEnum(TFR_EMoodPersonality))]
	protected TFR_EMoodPersonality m_ePersonality;

	[Attribute("0", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebug;

	[Attribute("1", UIWidgets.CheckBox, "Run Server Only")]
	protected bool m_bRunServerOnly;

	[Attribute("3000", UIWidgets.EditBox, "Mood Tick Interval MS")]
	protected int m_iMoodTickIntervalMs;

	[Attribute("4000", UIWidgets.EditBox, "Passive Stimulus Interval MS")]
	protected int m_iPassiveStimulusIntervalMs;

	[Attribute("2500", UIWidgets.EditBox, "Scavenger Tick Interval MS")]
	protected int m_iScavengerTickIntervalMs;

	[Attribute("5.0", UIWidgets.EditBox, "Ground Weapon Search Radius")]
	protected float m_fScavengerRadius;

	[Attribute("45", UIWidgets.EditBox, "Minimum Hostile Time Seconds")]
	protected float m_fHostileLockSeconds;

	[Attribute("0.15", UIWidgets.EditBox, "Nervous Threshold")]
	protected float m_fNervousThreshold;

	[Attribute("0.30", UIWidgets.EditBox, "Scared Threshold")]
	protected float m_fScaredThreshold;

	[Attribute("0.55", UIWidgets.EditBox, "Aggressive Threshold")]
	protected float m_fAggressiveThreshold;

	protected IEntity m_Owner;

	protected float m_fMoodIntensity = 0.0;
	protected TFR_EMoodState m_eCurrentState = TFR_EMoodState.CALM;

	protected bool m_bInitialized = false;

	protected bool m_bTFR_HostileLockActive = false;
	protected int m_iTFR_HostileLockUntil = 0;
	protected int m_iTFR_LastHostileLockLog = 0;
	protected int m_iTFR_LastHostileLockMaintainLog = 0;

	protected bool m_bTFR_HasAssignedWeapon = false;
	protected bool m_bTFR_ScavengerEnabled = false;
	protected ref map<IEntity, int> m_mTFR_FailedScavengerItems = new map<IEntity, int>();

	protected int m_iLastCalmLog = 0;
	protected int m_iLastScavengerFailLog = 0;
	protected int m_iNextPassiveStimulusTick = 0;

	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		m_Owner = owner;

		if (!m_Owner)
			return;

		if (!TFR_CanRunOnThisMachine())
		{
			TFR_DebugLog("[TFR MOOD] Init skipped on client. Logic reserved for server/host.");
			return;
		}

		if (m_iMoodTickIntervalMs <= 0)
			m_iMoodTickIntervalMs = 3000;

		if (m_iPassiveStimulusIntervalMs <= 0)
			m_iPassiveStimulusIntervalMs = 4000;

		if (m_iScavengerTickIntervalMs <= 0)
			m_iScavengerTickIntervalMs = 2500;

		if (m_fScavengerRadius <= 0)
			m_fScavengerRadius = 5.0;

		if (m_fHostileLockSeconds <= 0)
			m_fHostileLockSeconds = 45.0;

		ApplyPersonalityPreset();

		m_bInitialized = true;

		TFR_DebugLog("[TFR MOOD] Personality=" + m_ePersonality);
		TFR_DebugLog("[TFR MOOD] Initialized for entity=" + m_Owner);

		GetGame().GetCallqueue().Remove(TFR_MoodTick);
		GetGame().GetCallqueue().CallLater(TFR_MoodTick, m_iMoodTickIntervalMs, true);
	}

	override void OnDelete(IEntity owner)
	{
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(TFR_MoodTick);
			GetGame().GetCallqueue().Remove(TFR_ScavengerTick);
		}

		super.OnDelete(owner);
	}

	protected void ApplyPersonalityPreset()
	{
		if (m_ePersonality == TFR_EMoodPersonality.REACTIVE)
		{
			if (m_fNervousThreshold <= 0)
				m_fNervousThreshold = 0.15;

			if (m_fScaredThreshold <= 0)
				m_fScaredThreshold = 0.30;

			if (m_fAggressiveThreshold <= 0)
				m_fAggressiveThreshold = 0.55;

			return;
		}

		if (m_ePersonality == TFR_EMoodPersonality.CALM)
		{
			m_fNervousThreshold = 0.30;
			m_fScaredThreshold = 0.55;
			m_fAggressiveThreshold = 0.80;
		}
	}

	protected void TFR_MoodTick()
	{
		if (!m_bInitialized)
			return;

		if (!m_Owner)
			return;

		if (!TFR_CanRunOnThisMachine())
			return;

		TFR_ApplyPassiveStimulus();
		TFR_ApplyAutoCalm();
		TFR_UpdateMoodState();
	}

	protected void TFR_ApplyPassiveStimulus()
	{
		int now = System.GetTickCount();

		if (now < m_iNextPassiveStimulusTick)
			return;

		m_iNextPassiveStimulusTick = now + m_iPassiveStimulusIntervalMs;

		int randomRoll = Math.RandomInt(0, 100);

		if (randomRoll < 20)
		{
			AddMoodIntensity(0.16, "Passive tension");
			return;
		}

		if (randomRoll >= 20 && randomRoll < 35)
		{
			AddMoodIntensity(0.12, "Nervous reaction");
			return;
		}

		if (randomRoll >= 35 && randomRoll < 55)
		{
			ReduceMoodIntensity(0.06, "Calming down");
			return;
		}

		if (randomRoll >= 55 && randomRoll < 70)
		{
			ReduceMoodIntensity(0.024, "Breathing");
			return;
		}
	}

	protected void TFR_ApplyAutoCalm()
	{
		if (m_fMoodIntensity <= 0)
			return;

		float oldValue = m_fMoodIntensity;

		if (TFR_IsHostileLockValid(false))
			m_fMoodIntensity = Math.Max(0.0, m_fMoodIntensity - 0.0036);
		else
			m_fMoodIntensity = Math.Max(0.0, m_fMoodIntensity - 0.0096);

		int now = System.GetTickCount();

		if (m_bDebug && now - m_iLastCalmLog > 3000)
		{
			Print("[TFR MOOD] Auto calm: " + oldValue + " -> " + m_fMoodIntensity, LogLevel.NORMAL);
			m_iLastCalmLog = now;
		}
	}

	void AddMoodIntensity(float amount, string reason = "")
	{
		if (amount <= 0)
			return;

		float oldValue = m_fMoodIntensity;
		m_fMoodIntensity = Math.Clamp(m_fMoodIntensity + amount, 0.0, 1.0);

		if (reason != "")
			TFR_DebugLog("[TFR MOOD] Intensity: " + oldValue + " -> " + m_fMoodIntensity + " | " + reason);
		else
			TFR_DebugLog("[TFR MOOD] Intensity: " + oldValue + " -> " + m_fMoodIntensity);

		TFR_UpdateMoodState();
	}

	void ReduceMoodIntensity(float amount, string reason = "")
	{
		if (amount <= 0)
			return;

		float oldValue = m_fMoodIntensity;
		m_fMoodIntensity = Math.Clamp(m_fMoodIntensity - amount, 0.0, 1.0);

		if (reason != "")
			TFR_DebugLog("[TFR MOOD] Intensity: " + oldValue + " -> " + m_fMoodIntensity + " | " + reason);
		else
			TFR_DebugLog("[TFR MOOD] Intensity: " + oldValue + " -> " + m_fMoodIntensity);

		TFR_UpdateMoodState();
	}

	protected void TFR_UpdateMoodState()
	{
		TFR_EMoodState newState = TFR_EMoodState.CALM;

		if (m_fMoodIntensity >= m_fAggressiveThreshold)
			newState = TFR_EMoodState.AGGRESSIVE;
		else if (m_fMoodIntensity >= m_fScaredThreshold)
			newState = TFR_EMoodState.SCARED;
		else if (m_fMoodIntensity >= m_fNervousThreshold)
			newState = TFR_EMoodState.NERVOUS;
		else
			newState = TFR_EMoodState.CALM;

		if (TFR_IsHostileLockValid(false))
		{
			if (newState != TFR_EMoodState.AGGRESSIVE)
			{
				newState = TFR_EMoodState.AGGRESSIVE;

				int nowMaintain = System.GetTickCount();
				if (m_bDebug && nowMaintain - m_iTFR_LastHostileLockMaintainLog > 10000)
				{
					Print("[TFR MOOD] Hostile lock maintains aggressive state.", LogLevel.NORMAL);
					m_iTFR_LastHostileLockMaintainLog = nowMaintain;
				}
			}
		}

		if (newState == m_eCurrentState)
		{
			if (TFR_IsHostileLockValid(false))
				TFR_SetRealFaction("USSR", "Hostile lock keeps USSR.");

			return;
		}

		TFR_EMoodState oldState = m_eCurrentState;
		m_eCurrentState = newState;

		TFR_DebugLog("[TFR MOOD] State changed: " + TFR_StateToString(oldState) + " -> " + TFR_StateToString(newState));

		if (newState == TFR_EMoodState.CALM)
		{
			if (!TFR_IsHostileLockValid(false))
			{
				TFR_StopScavenger();
				TFR_SetRealFaction("FIA", "Calm");
			}

			return;
		}

		if (newState == TFR_EMoodState.NERVOUS)
		{
			if (!TFR_IsHostileLockValid(false))
			{
				TFR_StopScavenger();
				TFR_SetRealFaction("CIV", "Nervous");
			}

			return;
		}

		if (newState == TFR_EMoodState.SCARED)
		{
			if (!TFR_IsHostileLockValid(false))
			{
				TFR_StopScavenger();
				TFR_SetRealFaction("CIV", "Scared");
			}
			else
			{
				TFR_SetRealFaction("USSR", "Hostile lock keeps USSR.");
			}

			return;
		}

		if (newState == TFR_EMoodState.AGGRESSIVE)
		{
			TFR_ActivateHostileLock();
			TFR_SetRealFaction("USSR", "Aggressive");
			TFR_StartScavenger();
			return;
		}
	}

	protected string TFR_StateToString(TFR_EMoodState state)
	{
		if (state == TFR_EMoodState.CALM)
			return "Calm";

		if (state == TFR_EMoodState.NERVOUS)
			return "Nervous";

		if (state == TFR_EMoodState.SCARED)
			return "Scared";

		if (state == TFR_EMoodState.AGGRESSIVE)
			return "Aggressive";

		return "Unknown";
	}

	protected void TFR_SetRealFaction(string factionKey, string reason = "")
	{
		if (!m_Owner)
			return;

		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(m_Owner.FindComponent(FactionAffiliationComponent));

		if (!factionComp)
			return;

		Faction currentFaction = factionComp.GetAffiliatedFaction();
		string currentKey = "";

		if (currentFaction)
			currentKey = currentFaction.GetFactionKey();

		if (currentKey == factionKey)
			return;

		factionComp.SetAffiliatedFactionByKey(factionKey);

		if (reason != "")
			TFR_DebugLog("[TFR MOOD] Real faction changed: " + currentKey + " -> " + factionKey + " (" + reason + ")");
		else
			TFR_DebugLog("[TFR MOOD] Real faction changed: " + currentKey + " -> " + factionKey);
	}

	protected void TFR_ActivateHostileLock()
	{
		int now = System.GetTickCount();

		m_bTFR_HostileLockActive = true;
		m_iTFR_HostileLockUntil = now + Math.Round(m_fHostileLockSeconds * 1000.0);

		if (m_bDebug && now - m_iTFR_LastHostileLockLog > 10000)
		{
			Print("[TFR MOOD] Hostile lock activated. Minimum time ms=" + Math.Round(m_fHostileLockSeconds * 1000.0), LogLevel.NORMAL);
			m_iTFR_LastHostileLockLog = now;
		}
	}

	protected bool TFR_IsHostileLockValid(bool printExpire = true)
	{
		if (!m_bTFR_HostileLockActive)
			return false;

		int now = System.GetTickCount();

		if (now < m_iTFR_HostileLockUntil)
			return true;

		m_bTFR_HostileLockActive = false;

		if (printExpire)
			TFR_DebugLog("[TFR MOOD] Hostile lock expired.");

		return false;
	}

	protected void TFR_StartScavenger()
	{
		if (m_bTFR_HasAssignedWeapon)
			return;

		if (m_bTFR_ScavengerEnabled)
			return;

		m_bTFR_ScavengerEnabled = true;

		TFR_DebugLog("[TFR MOOD][SCAV] Scavenger enabled.");

		GetGame().GetCallqueue().Remove(TFR_ScavengerTick);
		GetGame().GetCallqueue().CallLater(TFR_ScavengerTick, m_iScavengerTickIntervalMs, true);
	}

	protected void TFR_StopScavenger()
	{
		if (!m_bTFR_ScavengerEnabled)
			return;

		m_bTFR_ScavengerEnabled = false;

		GetGame().GetCallqueue().Remove(TFR_ScavengerTick);

		TFR_DebugLog("[TFR MOOD][SCAV] Scavenger disabled.");
	}

	protected void TFR_ScavengerTick()
	{
		if (!m_bTFR_ScavengerEnabled)
			return;

		if (!TFR_CanRunOnThisMachine())
			return;

		if (m_bTFR_HasAssignedWeapon)
		{
			TFR_StopScavenger();
			return;
		}

		if (!m_Owner)
			return;

		vector pos = m_Owner.GetOrigin();
		float radius = m_fScavengerRadius;

		vector min = pos - Vector(radius, radius, radius);
		vector max = pos + Vector(radius, radius, radius);

		BaseWorld world = GetGame().GetWorld();

		if (!world)
			return;

		world.QueryEntitiesByAABB(min, max, TFR_OnFoundScavengerEntity, null);
	}

	protected bool TFR_OnFoundScavengerEntity(IEntity ent)
	{
		if (!ent)
			return true;

		if (!m_Owner)
			return false;

		if (m_bTFR_HasAssignedWeapon)
			return false;

		if (!TFR_CanRetryScavengerItem(ent))
			return true;

		if (!TFR_IsValidScavengerWeapon(ent))
			return true;

		TFR_DebugLog("[TFR MOOD][SCAV] Ground weapon found: " + ent);

		TFR_TryPickupScavengerItem(ent);

		return true;
	}

	protected bool TFR_IsValidScavengerWeapon(IEntity ent)
	{
		if (!ent)
			return false;

		if (ent == m_Owner)
			return false;

		string entStr = ent.ToString();
		entStr.ToLower();

		if (entStr.Contains("warhead")) return false;
		if (entStr.Contains("exiteffect")) return false;
		if (entStr.Contains("exit_effect")) return false;
		if (entStr.Contains("effect")) return false;
		if (entStr.Contains("grenade")) return false;
		if (entStr.Contains("rocket")) return false;
		if (entStr.Contains("shell")) return false;
		if (entStr.Contains("projectile")) return false;
		if (entStr.Contains("muzzle")) return false;
		if (entStr.Contains("particle")) return false;
		if (entStr.Contains("decal")) return false;
		if (entStr.Contains("sound")) return false;
		if (!entStr.Contains("rifle")) return false;

		WeaponComponent weapon = WeaponComponent.Cast(ent.FindComponent(WeaponComponent));

		if (!weapon)
			return false;

		return true;
	}

	protected bool TFR_IsValidScavengerMagazine(IEntity ent)
	{
		if (!ent)
			return false;

		if (ent == m_Owner)
			return false;

		string entStr = ent.ToString();
		entStr.ToLower();

		if (!entStr.Contains("magazine")) return false;
		if (entStr.Contains("warhead")) return false;
		if (entStr.Contains("effect")) return false;
		if (entStr.Contains("exiteffect")) return false;
		if (entStr.Contains("exit_effect")) return false;

		return true;
	}

	protected bool TFR_CanRetryScavengerItem(IEntity item)
	{
		if (!item)
			return false;

		if (!m_mTFR_FailedScavengerItems)
			m_mTFR_FailedScavengerItems = new map<IEntity, int>();

		if (!m_mTFR_FailedScavengerItems.Contains(item))
			return true;

		int retryAt = m_mTFR_FailedScavengerItems.Get(item);
		int now = System.GetTickCount();

		if (now < retryAt)
			return false;

		m_mTFR_FailedScavengerItems.Remove(item);
		return true;
	}

	protected void TFR_MarkFailedScavengerItem(IEntity item)
	{
		if (!item)
			return;

		if (!m_mTFR_FailedScavengerItems)
			m_mTFR_FailedScavengerItems = new map<IEntity, int>();

		m_mTFR_FailedScavengerItems.Set(item, System.GetTickCount() + 10000);
	}

	protected void TFR_TryPickupScavengerItem(IEntity item)
	{
		if (!item)
			return;

		if (!m_Owner)
			return;

		if (m_bTFR_HasAssignedWeapon)
			return;

		if (!TFR_CanRetryScavengerItem(item))
			return;

		TFR_DebugLog("[TFR MOOD][SCAV] Trying to equip weapon: " + item);

		bool success = TFR_TryInsertItemIntoInventory(item);

		if (!success)
		{
			int now = System.GetTickCount();

			if (m_bDebug && now - m_iLastScavengerFailLog > 3000)
			{
				Print("[TFR MOOD][SCAV] Could not insert item: " + item, LogLevel.NORMAL);
				m_iLastScavengerFailLog = now;
			}

			TFR_MarkFailedScavengerItem(item);
			return;
		}

		m_bTFR_HasAssignedWeapon = true;

		TFR_DebugLog("[TFR MOOD][SCAV] Weapon assigned. Scavenger disabled for this NPC.");

		TFR_StopScavenger();
	}

	protected bool TFR_TryInsertItemIntoInventory(IEntity item)
	{
		if (!item)
			return false;

		if (!m_Owner)
			return false;

		InventoryStorageManagerComponent invManager = InventoryStorageManagerComponent.Cast(m_Owner.FindComponent(InventoryStorageManagerComponent));

		if (!invManager)
			return false;

		bool inserted = invManager.TryInsertItem(item);

		if (!inserted)
			return false;

		return true;
	}

	void TFR_ForceAgro()
	{
		AddMoodIntensity(1.0, "Forced aggressive");
	}

	void TFR_ForceCalm()
	{
		m_fMoodIntensity = 0.0;
		m_bTFR_HostileLockActive = false;
		m_iTFR_HostileLockUntil = 0;

		TFR_StopScavenger();
		TFR_SetRealFaction("FIA", "ForceCalm");

		TFR_EMoodState oldState = m_eCurrentState;
		m_eCurrentState = TFR_EMoodState.CALM;

		TFR_DebugLog("[TFR MOOD] State changed: " + TFR_StateToString(oldState) + " -> Calm");
	}

	float TFR_GetMoodIntensity()
	{
		return m_fMoodIntensity;
	}

	TFR_EMoodState TFR_GetMoodState()
	{
		return m_eCurrentState;
	}

	bool TFR_IsAggressive()
	{
		return m_eCurrentState == TFR_EMoodState.AGGRESSIVE;
	}

	bool TFR_HasAssignedWeapon()
	{
		return m_bTFR_HasAssignedWeapon;
	}

	protected bool TFR_CanRunOnThisMachine()
	{
		if (!m_bRunServerOnly)
			return true;

		if (!Replication.IsRunning())
			return true;

		return Replication.IsServer();
	}

	protected void TFR_DebugLog(string message)
	{
		if (!m_bDebug)
			return;

		Print(message, LogLevel.NORMAL);
	}
}
