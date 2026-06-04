//------------------------------------------------------------------------------------------------
// TFR_MissionSettingAction.c
// TFR OPERATIONS
//
// Acciones admin para controlar persistencia desde un prefab con ActionManagerComponent.
// Pensado para radio/admin console.
//------------------------------------------------------------------------------------------------

class TFR_BasePersistenceSettingAction : ScriptedUserAction
{
	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("true", UIWidgets.CheckBox, "Require Server")]
	protected bool m_bRequireServer;

	protected bool CanRunAction()
	{
		if (!m_bEnabled)
			return false;

		if (!GetGame())
			return false;

		if (m_bRequireServer)
		{
			if (Replication.IsRunning() && !Replication.IsServer())
				return false;
		}

		TFR_PersistenceManagerComponent persistence = TFR_PersistenceManagerComponent.GetActivePersistence();

		if (!persistence)
			return false;

		return true;
	}

	protected TFR_PersistenceManagerComponent GetPersistence()
	{
		return TFR_PersistenceManagerComponent.GetActivePersistence();
	}

	override bool CanBeShownScript(IEntity user)
	{
		return CanRunAction();
	}

	override bool CanBePerformedScript(IEntity user)
	{
		return CanRunAction();
	}

	protected void AdminLog(string text)
	{
		Print("[TFR Mission Settings Action] " + text, LogLevel.NORMAL);
	}
}

class TFR_SaveOperationNowAction : TFR_BasePersistenceSettingAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_PersistenceManagerComponent persistence = GetPersistence();

		if (!persistence)
			return;

		persistence.SaveOperationNow();
		AdminLog("SaveOperationNow executed.");
	}
}

class TFR_LoadOperationNowAction : TFR_BasePersistenceSettingAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_PersistenceManagerComponent persistence = GetPersistence();

		if (!persistence)
			return;

		persistence.LoadOperationNow();
		AdminLog("LoadOperationNow executed.");
	}
}

class TFR_ResetOperationPersistenceAction : TFR_BasePersistenceSettingAction
{
	[Attribute("true", UIWidgets.CheckBox, "Restart Operation After Reset")]
	protected bool m_bRestartOperationAfterReset;

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_PersistenceManagerComponent persistence = GetPersistence();

		if (!persistence)
			return;

		persistence.ResetOperationPersistence(m_bRestartOperationAfterReset);
		AdminLog("ResetOperationPersistence executed. Restart=" + m_bRestartOperationAfterReset.ToString());
	}
}

class TFR_ResetPlayerInventoryPersistenceAction : TFR_BasePersistenceSettingAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_PersistenceManagerComponent persistence = GetPersistence();

		if (!persistence)
			return;

		persistence.ResetPlayerInventoryPersistence();
		AdminLog("ResetPlayerInventoryPersistence executed.");
	}
}

class TFR_ResetVehicleCargoPersistenceAction : TFR_BasePersistenceSettingAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_PersistenceManagerComponent persistence = GetPersistence();

		if (!persistence)
			return;

		persistence.ResetVehicleCargoPersistence();
		AdminLog("ResetVehicleCargoPersistence executed.");
	}
}

class TFR_ResetAllPersistenceAction : TFR_BasePersistenceSettingAction
{
	[Attribute("true", UIWidgets.CheckBox, "Restart Operation After Reset")]
	protected bool m_bRestartOperationAfterReset;

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_PersistenceManagerComponent persistence = GetPersistence();

		if (!persistence)
			return;

		persistence.ResetAllPersistence(m_bRestartOperationAfterReset);
		AdminLog("ResetAllPersistence executed. Restart=" + m_bRestartOperationAfterReset.ToString());
	}
}

class TFR_PrintPersistenceStateAction : TFR_BasePersistenceSettingAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		TFR_PersistenceManagerComponent persistence = GetPersistence();

		if (!persistence)
			return;

		persistence.TFR_AdminPrintPersistenceState();
		AdminLog("TFR_AdminPrintPersistenceState executed.");
	}
}
