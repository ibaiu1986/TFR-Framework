//------------------------------------------------------------------------------------------------
// TFR_OperationsRadialEntry.c
// TFR OPERATIONS
//
// Entrada del menú radial TFR Operations.
//
// Versión limpia PC / PS5 / server:
//
// - Solo ejecuta desde OnPerform().
// - No usa OnEntryClick().
// - No usa InvokeOnPerform() manual.
// - No cierra manualmente el radial.
// - El cierre lo hace el propio radial con Close On Perform = true.
// - HALO abre el mapa con pequeño delay para dejar respirar al radial.
//------------------------------------------------------------------------------------------------

enum TFR_EOperationsRadialEntryType
{
	HALO_MAP = 0,

	MISSION_PRINT_STATE = 10,
	MISSION_FORCE_NEXT_INTEL = 11,
	MISSION_COMPLETE = 12,
	MISSION_FAIL = 13,
	MISSION_RESTART_INTEL = 14,

	PERSISTENCE_SAVE_OPERATION = 30,
	PERSISTENCE_LOAD_OPERATION = 31,
	PERSISTENCE_RESET_OPERATION = 32,
	PERSISTENCE_RESET_PLAYER_INVENTORY = 33,
	PERSISTENCE_RESET_VEHICLE_CARGO = 34,
	PERSISTENCE_RESET_ALL = 35,
	PERSISTENCE_PRINT_STATE = 36,

	PLAYER_INVENTORY_SAVE_NOW = 50,
	PLAYER_INVENTORY_RESTORE_NOW = 51,
	PLAYER_INVENTORY_PRINT_STATE = 52,
	PLAYER_INVENTORY_RESET = 53
}

//------------------------------------------------------------------------------------------------
class TFR_OperationsRadialEntry : SCR_SelectionMenuEntry
{
	protected TFR_EOperationsRadialEntryType m_eEntryType;
	protected string m_sHintTitle;
	protected float m_fHintDuration;

	protected const int TFR_HALO_OPEN_DELAY_MS = 150;
	protected const int TFR_HALO_RETRY_COUNT = 12;
	protected const int TFR_HALO_RETRY_DELAY_MS = 150;

	//------------------------------------------------------------------------------------------------
	void TFR_OperationsRadialEntry(
		TFR_EOperationsRadialEntryType entryType,
		string name,
		string description,
		string hintTitle,
		float hintDuration
	)
	{
		m_eEntryType = entryType;
		m_sHintTitle = hintTitle;
		m_fHintDuration = hintDuration;

		if (m_sHintTitle == string.Empty)
			m_sHintTitle = "TFR Operations";

		if (m_fHintDuration <= 0)
			m_fHintDuration = 4.0;

		SetId(name);
		SetName(name);
		SetDescription(description);
		Enable(true);
	}

	//------------------------------------------------------------------------------------------------
	override void OnPerform()
	{
		Print("[TFR Operations Radial] Entry performed. Action=" + m_eEntryType.ToString(), LogLevel.NORMAL);

		if (m_eEntryType == TFR_EOperationsRadialEntryType.HALO_MAP)
		{
			if (GetGame())
				GetGame().GetCallqueue().CallLater(TFR_StartHaloMapSelectionDelayed, TFR_HALO_OPEN_DELAY_MS, false);
			else
				TFR_StartHaloMapSelectionDelayed();

			super.OnPerform();
			return;
		}

		TFR_SendNetworkAction();

		super.OnPerform();
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_SendNetworkAction()
	{
		TFR_OperationsRadialNetworkComponent networkComponent = TFR_GetOperationsNetworkComponent();

		if (!networkComponent)
		{
			TFR_ShowHint("Esperando conexión con servidor...");
			Print("[TFR Operations Radial] Network component not found for radial action. Action=" + m_eEntryType.ToString(), LogLevel.WARNING);
			return;
		}

		Print("[TFR Operations Radial] Sending radial action through network component. Action=" + m_eEntryType.ToString(), LogLevel.NORMAL);

		networkComponent.TFR_RequestRadialAction(
			m_eEntryType,
			m_sHintTitle,
			m_fHintDuration
		);
	}

	//------------------------------------------------------------------------------------------------
	protected TFR_OperationsRadialNetworkComponent TFR_GetOperationsNetworkComponent()
	{
		if (!GetGame())
			return null;

		PlayerController playerController = GetGame().GetPlayerController();

		if (playerController)
		{
			TFR_OperationsRadialNetworkComponent networkOnController = TFR_OperationsRadialNetworkComponent.Cast(
				playerController.FindComponent(TFR_OperationsRadialNetworkComponent)
			);

			if (networkOnController)
				return networkOnController;
		}

		IEntity controlledEntity = TFR_GetLocalControlledEntity();

		if (controlledEntity)
		{
			TFR_OperationsRadialNetworkComponent networkOnCharacter = TFR_OperationsRadialNetworkComponent.Cast(
				controlledEntity.FindComponent(TFR_OperationsRadialNetworkComponent)
			);

			if (networkOnCharacter)
				return networkOnCharacter;
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_StartHaloMapSelectionDelayed()
	{
		IEntity userEntity = TFR_GetLocalControlledEntity();

		if (!userEntity)
		{
			TFR_ShowHint("HALO ERROR: usuario no válido");
			Print("[TFR Operations Radial] HALO blocked: no local controlled entity.", LogLevel.WARNING);
			return;
		}

		TFR_TryBeginHaloMapSelection(userEntity, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_TryBeginHaloMapSelection(IEntity userEntity, int attempt)
	{
		if (!userEntity)
		{
			TFR_ShowHint("HALO ERROR: usuario no válido");
			Print("[TFR Operations Radial] HALO retry blocked: user null. Attempt=" + attempt.ToString(), LogLevel.WARNING);
			return;
		}

		TFR_HaloMapSelectionComponent selector = TFR_HaloMapSelectionComponent.GetLocalSelectionComponent(userEntity);

		if (!selector)
		{
			if (attempt == 0)
				TFR_ShowHint("Buscando sistema HALO por mapa...");

			if (attempt >= TFR_HALO_RETRY_COUNT)
			{
				TFR_ShowHint("HALO ERROR: no se encontró el sistema de selección por mapa");
				Print("[TFR Operations Radial] HALO map selector not found after retries.", LogLevel.WARNING);
				return;
			}

			if (GetGame())
			{
				GetGame().GetCallqueue().CallLater(
					TFR_TryBeginHaloMapSelection,
					TFR_HALO_RETRY_DELAY_MS,
					false,
					userEntity,
					attempt + 1
				);
			}

			return;
		}

		if (!selector.BeginHaloMapSelection())
		{
			if (attempt == 0)
				TFR_ShowHint("Preparando selección HALO por mapa...");

			if (attempt >= TFR_HALO_RETRY_COUNT)
			{
				TFR_ShowHint("HALO ERROR: no se pudo abrir la selección por mapa");
				Print("[TFR Operations Radial] HALO map selection failed after retries.", LogLevel.WARNING);
				return;
			}

			if (GetGame())
			{
				GetGame().GetCallqueue().CallLater(
					TFR_TryBeginHaloMapSelection,
					TFR_HALO_RETRY_DELAY_MS,
					false,
					userEntity,
					attempt + 1
				);
			}

			return;
		}

		Print("[TFR Operations Radial] HALO map selection opened successfully. Attempt=" + attempt.ToString(), LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntity TFR_GetLocalControlledEntity()
	{
		if (!GetGame())
			return null;

		PlayerController playerController = GetGame().GetPlayerController();

		if (playerController)
		{
			SCR_PlayerController scrPlayerController = SCR_PlayerController.Cast(playerController);

			if (scrPlayerController)
			{
				IEntity controlledFromController = scrPlayerController.GetControlledEntity();

				if (controlledFromController)
					return controlledFromController;
			}

			PlayerManager playerManager = GetGame().GetPlayerManager();

			if (playerManager)
			{
				int playerId = playerController.GetPlayerId();

				if (playerId > 0)
				{
					IEntity controlledFromManager = playerManager.GetPlayerControlledEntity(playerId);

					if (controlledFromManager)
						return controlledFromManager;
				}
			}
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_ShowHint(string message)
	{
		if (message == string.Empty)
			return;

		SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();

		if (hintMgr)
			hintMgr.ShowCustomHint(message, m_sHintTitle, m_fHintDuration);
	}
}
