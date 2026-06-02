//------------------------------------------------------------------------------------------------
// TFR_OperationsRadialNetworkComponent.c
// TFR OPERATIONS
//
// Puente de red para acciones del radial TFR Operations.
//
// RECOMENDADO:
// - En PlayerController.
//
// TOLERADO:
// - En personaje controlado, si el prefab real ya lo tiene ahí.
//
// - Cliente solicita acción.
// - Servidor valida jugador/facción.
// - Servidor ejecuta MissionManager/PersistenceManager.
// - Servidor responde con hint solo al dueño.
//
// Incluye API estática para ejecución directa en Workbench/listen server.
//
// SERVER POLISH:
// - El BOOT log y logs normales pasan a Debug Logs.
// - Warnings importantes siguen visibles.
// - No cambia RPC.
// - No cambia validación de facción.
// - No cambia ejecución de misión/persistencia.
//------------------------------------------------------------------------------------------------

[ComponentEditorProps(category: "TFR Operations/Radial", description: "Network bridge for TFR Operations radial actions.")]
class TFR_OperationsRadialNetworkComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class TFR_OperationsRadialNetworkComponent : ScriptComponent
{
	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebug;

	[Attribute("true", UIWidgets.CheckBox, "Require Authorized Faction")]
	protected bool m_bRequireAuthorizedFaction;

	[Attribute("US", UIWidgets.EditBox, "Authorized Faction Key")]
	protected string m_sAuthorizedFactionKey;

	[Attribute("true", UIWidgets.CheckBox, "Allow Mission Admin Actions")]
	protected bool m_bAllowMissionAdminActions;

	[Attribute("true", UIWidgets.CheckBox, "Allow Persistence Actions")]
	protected bool m_bAllowPersistenceActions;

	[Attribute("true", UIWidgets.CheckBox, "Restart Operation After Reset Operation")]
	protected bool m_bRestartOperationAfterResetOperation;

	[Attribute("true", UIWidgets.CheckBox, "Restart Operation After Reset All")]
	protected bool m_bRestartOperationAfterResetAll;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		TFR_DebugLog("Component reached. IsServer=" + Replication.IsServer().ToString() + " IsConsoleApp=" + System.IsConsoleApp().ToString() + " Owner=" + owner.ToString());
	}

	//------------------------------------------------------------------------------------------------
	void TFR_RequestRadialAction(TFR_EOperationsRadialEntryType entryType, string hintTitle, float hintDuration)
	{
		if (!m_bEnabled)
		{
			TFR_ShowLocalHint("Sistema radial TFR desactivado.", hintTitle, hintDuration);
			return;
		}

		if (hintTitle == string.Empty)
			hintTitle = "TFR Operations";

		if (hintDuration <= 0)
			hintDuration = 4.0;

		TFR_DebugLog("Client/request side action=" + entryType.ToString());

		if (Replication.IsServer())
		{
			IEntity localUser = TFR_GetLocalControlledEntitySafe();

			string directResult = TFR_ExecuteRadialActionStatic(
				entryType,
				localUser,
				m_bRequireAuthorizedFaction,
				m_sAuthorizedFactionKey,
				m_bAllowMissionAdminActions,
				m_bAllowPersistenceActions,
				m_bRestartOperationAfterResetOperation,
				m_bRestartOperationAfterResetAll
			);

			if (directResult == string.Empty)
				directResult = "Acción TFR ejecutada.";

			TFR_ShowLocalHint(directResult, hintTitle, hintDuration);
			return;
		}

		Rpc(RpcAsk_TFR_RequestRadialAction, entryType, hintTitle, hintDuration);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_TFR_RequestRadialAction(TFR_EOperationsRadialEntryType entryType, string hintTitle, float hintDuration)
	{
		if (!Replication.IsServer())
			return;

		if (!m_bEnabled)
		{
			Rpc(RpcDo_TFR_ShowHint, "Sistema radial TFR desactivado.", hintTitle, hintDuration);
			return;
		}

		int playerId = TFR_GetRequestingPlayerId();
		IEntity controlledEntity = null;

		if (playerId > 0)
			controlledEntity = TFR_GetControlledEntity(playerId);

		if (!controlledEntity)
			controlledEntity = TFR_GetOwnerAsEntityFallback();

		if (!controlledEntity)
		{
			Rpc(RpcDo_TFR_ShowHint, "Acción TFR bloqueada: jugador no disponible en servidor.", hintTitle, hintDuration);
			Print("[TFR Operations Radial Network] No controlled entity for radial request. PlayerId=" + playerId.ToString(), LogLevel.WARNING);
			return;
		}

		TFR_DebugLog("Server action received. PlayerId=" + playerId.ToString() + " Action=" + entryType.ToString());

		string resultMessage = TFR_ExecuteRadialActionStatic(
			entryType,
			controlledEntity,
			m_bRequireAuthorizedFaction,
			m_sAuthorizedFactionKey,
			m_bAllowMissionAdminActions,
			m_bAllowPersistenceActions,
			m_bRestartOperationAfterResetOperation,
			m_bRestartOperationAfterResetAll
		);

		if (resultMessage == string.Empty)
			resultMessage = "Acción TFR enviada.";

		Rpc(RpcDo_TFR_ShowHint, resultMessage, hintTitle, hintDuration);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_TFR_ShowHint(string message, string hintTitle, float hintDuration)
	{
		if (System.IsConsoleApp())
			return;

		TFR_ShowLocalHint(message, hintTitle, hintDuration);
	}

	//------------------------------------------------------------------------------------------------
	static string TFR_ExecuteRadialActionStatic(
		TFR_EOperationsRadialEntryType entryType,
		IEntity userEntity,
		bool requireAuthorizedFaction,
		string authorizedFactionKey,
		bool allowMissionAdminActions,
		bool allowPersistenceActions,
		bool restartOperationAfterResetOperation,
		bool restartOperationAfterResetAll
	)
	{
		if (!Replication.IsServer())
			return "Acción TFR no ejecutada: no es servidor.";

		if (!TFR_StaticCanUserAccess(userEntity, requireAuthorizedFaction, authorizedFactionKey))
			return "No tienes autorización para usar acciones TFR.";

		switch (entryType)
		{
			case TFR_EOperationsRadialEntryType.MISSION_PRINT_STATE:
				return TFR_StaticMissionPrintState(allowMissionAdminActions);

			case TFR_EOperationsRadialEntryType.MISSION_FORCE_NEXT_INTEL:
				return TFR_StaticMissionForceNextIntel(allowMissionAdminActions);

			case TFR_EOperationsRadialEntryType.MISSION_COMPLETE:
				return TFR_StaticMissionComplete(allowMissionAdminActions);

			case TFR_EOperationsRadialEntryType.MISSION_FAIL:
				return TFR_StaticMissionFail(allowMissionAdminActions);

			case TFR_EOperationsRadialEntryType.MISSION_RESTART_INTEL:
				return TFR_StaticMissionRestartIntel(allowMissionAdminActions);

			case TFR_EOperationsRadialEntryType.PERSISTENCE_SAVE_OPERATION:
				return TFR_StaticPersistenceSaveOperation(allowPersistenceActions);

			case TFR_EOperationsRadialEntryType.PERSISTENCE_LOAD_OPERATION:
				return TFR_StaticPersistenceLoadOperation(allowPersistenceActions);

			case TFR_EOperationsRadialEntryType.PERSISTENCE_RESET_OPERATION:
				return TFR_StaticPersistenceResetOperation(allowPersistenceActions, restartOperationAfterResetOperation);

			case TFR_EOperationsRadialEntryType.PERSISTENCE_RESET_PLAYER_INVENTORY:
				return TFR_StaticPersistenceResetPlayerInventory(allowPersistenceActions);

			case TFR_EOperationsRadialEntryType.PERSISTENCE_RESET_VEHICLE_CARGO:
				return TFR_StaticPersistenceResetVehicleCargo(allowPersistenceActions);

			case TFR_EOperationsRadialEntryType.PERSISTENCE_RESET_ALL:
				return TFR_StaticPersistenceResetAll(allowPersistenceActions, restartOperationAfterResetAll);

			case TFR_EOperationsRadialEntryType.PERSISTENCE_PRINT_STATE:
				return TFR_StaticPersistencePrintState(allowPersistenceActions);
		}

		return "Acción TFR no reconocida.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticMissionPrintState(bool allowed)
	{
		if (!allowed)
			return "Acciones admin de misión desactivadas.";

		TFR_MissionManagerComponent manager = TFR_MissionManagerComponent.GetActiveManager();

		if (!manager)
			return "MissionManager TFR no encontrado.";

		manager.TFR_AdminPrintState();
		return "Diagnóstico de misión enviado al log del servidor.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticMissionForceNextIntel(bool allowed)
	{
		if (!allowed)
			return "Acciones admin de misión desactivadas.";

		TFR_MissionManagerComponent manager = TFR_MissionManagerComponent.GetActiveManager();

		if (!manager)
			return "MissionManager TFR no encontrado.";

		manager.TFR_AdminForceNextIntel();
		return "Forzar siguiente intel ejecutado.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticMissionComplete(bool allowed)
	{
		if (!allowed)
			return "Acciones admin de misión desactivadas.";

		TFR_MissionManagerComponent manager = TFR_MissionManagerComponent.GetActiveManager();

		if (!manager)
			return "MissionManager TFR no encontrado.";

		manager.TFR_AdminCompleteMission();
		return "Completar misión ejecutado.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticMissionFail(bool allowed)
	{
		if (!allowed)
			return "Acciones admin de misión desactivadas.";

		TFR_MissionManagerComponent manager = TFR_MissionManagerComponent.GetActiveManager();

		if (!manager)
			return "MissionManager TFR no encontrado.";

		manager.TFR_AdminFailMission();
		return "Fallar misión ejecutado.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticMissionRestartIntel(bool allowed)
	{
		if (!allowed)
			return "Acciones admin de misión desactivadas.";

		TFR_MissionManagerComponent manager = TFR_MissionManagerComponent.GetActiveManager();

		if (!manager)
			return "MissionManager TFR no encontrado.";

		manager.TFR_AdminCancelAndRestartIntel();
		return "Reinicio de cadena intel ejecutado.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticPersistenceSaveOperation(bool allowed)
	{
		if (!allowed)
			return "Acciones de persistencia desactivadas.";

		TFR_PersistenceManagerComponent persistence = TFR_PersistenceManagerComponent.GetActivePersistence();

		if (!persistence)
			return "PersistenceManager TFR no encontrado.";

		persistence.SaveOperationNow();
		return "Guardado de operación ejecutado.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticPersistenceLoadOperation(bool allowed)
	{
		if (!allowed)
			return "Acciones de persistencia desactivadas.";

		TFR_PersistenceManagerComponent persistence = TFR_PersistenceManagerComponent.GetActivePersistence();

		if (!persistence)
			return "PersistenceManager TFR no encontrado.";

		persistence.LoadOperationNow();
		return "Carga de operación ejecutada.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticPersistenceResetOperation(bool allowed, bool restartOperation)
	{
		if (!allowed)
			return "Acciones de persistencia desactivadas.";

		TFR_PersistenceManagerComponent persistence = TFR_PersistenceManagerComponent.GetActivePersistence();

		if (!persistence)
			return "PersistenceManager TFR no encontrado.";

		persistence.ResetOperationPersistence(restartOperation);
		return "Reset de operación ejecutado.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticPersistenceResetPlayerInventory(bool allowed)
	{
		if (!allowed)
			return "Acciones de persistencia desactivadas.";

		TFR_PersistenceManagerComponent persistence = TFR_PersistenceManagerComponent.GetActivePersistence();

		if (!persistence)
			return "PersistenceManager TFR no encontrado.";

		persistence.ResetPlayerInventoryPersistence();
		return "Reset de inventario de jugadores ejecutado.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticPersistenceResetVehicleCargo(bool allowed)
	{
		if (!allowed)
			return "Acciones de persistencia desactivadas.";

		TFR_PersistenceManagerComponent persistence = TFR_PersistenceManagerComponent.GetActivePersistence();

		if (!persistence)
			return "PersistenceManager TFR no encontrado.";

		persistence.ResetVehicleCargoPersistence();
		return "Reset de vehículos ejecutado.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticPersistenceResetAll(bool allowed, bool restartOperation)
	{
		if (!allowed)
			return "Acciones de persistencia desactivadas.";

		TFR_PersistenceManagerComponent persistence = TFR_PersistenceManagerComponent.GetActivePersistence();

		if (!persistence)
			return "PersistenceManager TFR no encontrado.";

		persistence.ResetAllPersistence(restartOperation);
		return "Reset total de persistencia ejecutado.";
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticPersistencePrintState(bool allowed)
	{
		if (!allowed)
			return "Acciones de persistencia desactivadas.";

		TFR_PersistenceManagerComponent persistence = TFR_PersistenceManagerComponent.GetActivePersistence();

		if (!persistence)
			return "PersistenceManager TFR no encontrado.";

		persistence.TFR_AdminPrintPersistenceState();
		return "Diagnóstico de persistencia enviado al log del servidor.";
	}

	//------------------------------------------------------------------------------------------------
	protected static bool TFR_StaticCanUserAccess(IEntity userEntity, bool requireAuthorizedFaction, string authorizedFactionKey)
	{
		if (!requireAuthorizedFaction)
			return true;

		if (authorizedFactionKey == string.Empty)
			return true;

		if (!userEntity)
			return false;

		string factionKey = TFR_StaticGetFactionKey(userEntity);

		if (factionKey == string.Empty)
			return false;

		return factionKey == authorizedFactionKey;
	}

	//------------------------------------------------------------------------------------------------
	protected static string TFR_StaticGetFactionKey(IEntity ent)
	{
		if (!ent)
			return string.Empty;

		FactionAffiliationComponent factionComp = FactionAffiliationComponent.Cast(ent.FindComponent(FactionAffiliationComponent));

		if (!factionComp)
			return string.Empty;

		Faction faction = factionComp.GetAffiliatedFaction();

		if (!faction)
			return string.Empty;

		return faction.GetFactionKey();
	}

	//------------------------------------------------------------------------------------------------
	protected int TFR_GetRequestingPlayerId()
	{
		PlayerController playerController = PlayerController.Cast(GetOwner());

		if (playerController)
			return playerController.GetPlayerId();

		IEntity ownerEntity = IEntity.Cast(GetOwner());

		if (ownerEntity)
		{
			PlayerManager playerManager = GetGame().GetPlayerManager();

			if (playerManager)
				return playerManager.GetPlayerIdFromControlledEntity(ownerEntity);
		}

		return 0;
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity TFR_GetControlledEntity(int playerId)
	{
		if (playerId <= 0)
			return null;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return null;

		return playerManager.GetPlayerControlledEntity(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity TFR_GetOwnerAsEntityFallback()
	{
		return IEntity.Cast(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity TFR_GetLocalControlledEntitySafe()
	{
		if (!GetGame())
			return null;

		PlayerController playerController = GetGame().GetPlayerController();

		if (!playerController)
			return null;

		return playerController.GetControlledEntity();
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_ShowLocalHint(string message, string hintTitle, float hintDuration)
	{
		if (System.IsConsoleApp())
			return;

		if (message == string.Empty)
			return;

		if (hintTitle == string.Empty)
			hintTitle = "TFR Operations";

		if (hintDuration <= 0)
			hintDuration = 4.0;

		SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();

		if (hintMgr)
			hintMgr.ShowCustomHint(message, hintTitle, hintDuration);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_DebugLog(string message)
	{
		if (!m_bDebug)
			return;

		Print("[TFR Operations Radial Network] " + message, LogLevel.NORMAL);
	}
}
