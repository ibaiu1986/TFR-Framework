//------------------------------------------------------------------------------------------------
// TFR_HaloMapSelectionComponent.c
// TFR OPERATIONS
//
// HALO MAP SELECTION V1.3.1 CLEAN
//
// - Selección HALO por mapa.
// - Cliente elige punto.
// - Servidor valida, calcula y mueve.
// - Cliente dueño aplica también la posición aprobada por servidor.
// - Mantiene el fix funcional probado en servidor dedicado.
// - Sin mensajes debug visibles.
// - Sin EOnFrame.
// - Preparado para partida/testers.
//------------------------------------------------------------------------------------------------

class TFR_HaloMapSelectionComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class TFR_HaloMapSelectionComponent : ScriptComponent
{
	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	protected bool m_bSelectionActive;
	protected bool m_bWaitingServerHaloResponse;

	protected IEntity m_Owner;
	protected SCR_MapEntity m_MapEntity;

	protected const int TFR_MAP_INSTANCE_RETRY_COUNT = 12;
	protected const int TFR_MAP_INSTANCE_RETRY_DELAY_MS = 150;

	protected const int TFR_SERVER_RESPONSE_TIMEOUT_MS = 4000;
	protected const int TFR_REQUEST_AFTER_MAP_CLOSE_DELAY_MS = 1500;

	//------------------------------------------------------------------------------------------------
	void TFR_HaloMapSelectionComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		// No forzar valores del inspector.
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_Owner = owner;
		m_bSelectionActive = false;
		m_bWaitingServerHaloResponse = false;

		LogTFR("Map selection component initialized. Owner=" + owner.ToString());
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisableHaloMapSelection(false);

		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(RequestHaloJumpAtPosition);
			GetGame().GetCallqueue().Remove(TFR_TryBindMapSelection);
			GetGame().GetCallqueue().Remove(TFR_CheckServerHaloResponseTimeout);
			GetGame().GetCallqueue().Remove(TFR_ForceCloseMapUI);
			GetGame().GetCallqueue().Remove(TFR_ClientApplyHaloTeleport);
		}

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	static TFR_HaloMapSelectionComponent GetLocalSelectionComponent(IEntity userEntity)
	{
		if (!userEntity)
			return null;

		TFR_HaloMapSelectionComponent comp = TFR_HaloMapSelectionComponent.Cast(
			userEntity.FindComponent(TFR_HaloMapSelectionComponent)
		);

		if (comp)
			return comp;

		PlayerController localPlayerController = GetGame().GetPlayerController();

		if (!localPlayerController)
			return null;

		IEntity controlledEntity = localPlayerController.GetControlledEntity();

		if (!controlledEntity)
			return null;

		return TFR_HaloMapSelectionComponent.Cast(
			controlledEntity.FindComponent(TFR_HaloMapSelectionComponent)
		);
	}

	//------------------------------------------------------------------------------------------------
	bool BeginHaloMapSelection()
	{
		if (!m_bEnabled)
		{
			ShowHintLocal("HALO no disponible.", "TFR HALO", 4.0);
			return false;
		}

		if (!IsLocalControlledOwner())
		{
			ShowHintLocal("HALO no disponible para este jugador.", "TFR HALO", 4.0);
			LogTFR("Begin selection blocked: owner is not local controlled entity.");
			return false;
		}

		if (m_bSelectionActive)
		{
			ShowHintLocal("Selección HALO ya activa.", "TFR HALO", 3.0);
			LogTFR("Begin selection ignored: selection already active.");
			return true;
		}

		m_bSelectionActive = true;
		m_MapEntity = null;

		ShowHintLocal("Abriendo selección HALO por mapa.", "TFR HALO", 3.0);

		OpenMapMenu();

		TFR_TryBindMapSelection(0);

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_TryBindMapSelection(int attempt)
	{
		if (!m_bEnabled)
		{
			DisableHaloMapSelection(false);
			return;
		}

		if (!m_bSelectionActive)
			return;

		m_MapEntity = SCR_MapEntity.GetMapInstance();

		if (!m_MapEntity)
		{
			LogTFR("Waiting for SCR_MapEntity instance. Attempt=" + attempt.ToString());

			if (attempt >= TFR_MAP_INSTANCE_RETRY_COUNT)
			{
				ShowHintLocal("No se pudo abrir el mapa HALO.", "TFR HALO", 5.0);
				LogTFR("Begin selection blocked: no SCR_MapEntity instance after retries.");
				DisableHaloMapSelection(false);
				return;
			}

			GetGame().GetCallqueue().CallLater(
				TFR_TryBindMapSelection,
				TFR_MAP_INSTANCE_RETRY_DELAY_MS,
				false,
				attempt + 1
			);

			return;
		}

		SCR_MapEntity.GetOnSelection().Remove(OnMapSelection);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);

		SCR_MapEntity.GetOnSelection().Insert(OnMapSelection);
		SCR_MapEntity.GetOnMapClose().Insert(OnMapClose);

		ShowHintLocal(
			"Haz click en el mapa para elegir el punto exacto de inserción HALO.",
			"TFR HALO",
			6.0
		);

		LogTFR("HALO map selection started.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OpenMapMenu()
	{
		MenuManager menuManager = GetGame().GetMenuManager();

		if (!menuManager)
		{
			ShowHintLocal("No se pudo abrir el mapa.", "TFR HALO", 4.0);
			LogTFR("OpenMapMenu failed: no MenuManager.");
			return;
		}

		if (menuManager.FindMenuByPreset(ChimeraMenuPreset.MapMenu))
		{
			LogTFR("MapMenu already open.");
			return;
		}

		menuManager.OpenMenu(ChimeraMenuPreset.MapMenu);

		LogTFR("Requested MapMenu open.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapSelection(vector coords)
	{
		if (!m_bSelectionActive)
			return;

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (!mapEntity)
		{
			ShowHintLocal("Selección HALO cancelada: mapa no válido.", "TFR HALO", 4.0);
			LogTFR("Map selection ignored: no map entity.");
			DisableHaloMapSelection(true);
			return;
		}

		float worldX;
		float worldZ;

		mapEntity.ScreenToWorld(coords[0], coords[2], worldX, worldZ);

		vector dropZonePosition = vector.Zero;
		dropZonePosition[0] = worldX;
		dropZonePosition[2] = worldZ;

		if (dropZonePosition == vector.Zero)
		{
			ShowHintLocal("Punto HALO no válido.", "TFR HALO", 4.0);
			DisableHaloMapSelection(true);
			return;
		}

		LogTFR("Map selected drop zone=" + dropZonePosition.ToString());

		ShowHintLocal("Punto HALO seleccionado. Preparando inserción...", "TFR HALO", 4.0);

		DisableHaloMapSelection(false);

		TFR_ScheduleForceCloseMapUI();

		GetGame().GetCallqueue().CallLater(
			RequestHaloJumpAtPosition,
			TFR_REQUEST_AFTER_MAP_CLOSE_DELAY_MS,
			false,
			dropZonePosition
		);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_ScheduleForceCloseMapUI()
	{
		if (!GetGame())
			return;

		TFR_ForceCloseMapUI();

		GetGame().GetCallqueue().CallLater(TFR_ForceCloseMapUI, 100, false);
		GetGame().GetCallqueue().CallLater(TFR_ForceCloseMapUI, 300, false);
		GetGame().GetCallqueue().CallLater(TFR_ForceCloseMapUI, 700, false);
		GetGame().GetCallqueue().CallLater(TFR_ForceCloseMapUI, 1200, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_ForceCloseMapUI()
	{
		MenuManager menuManager = GetGame().GetMenuManager();

		if (menuManager)
		{
			MenuBase mapMenu = menuManager.FindMenuByPreset(ChimeraMenuPreset.MapMenu);

			if (mapMenu)
			{
				mapMenu.Close();
				LogTFR("Force close: MapMenu closed.");
			}
		}

		SCR_MapEntity mapEntity = SCR_MapEntity.GetMapInstance();

		if (mapEntity && mapEntity.IsOpen())
		{
			mapEntity.CloseMap();
			LogTFR("Force close: SCR_MapEntity closed.");
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMapClose(MapConfiguration config)
	{
		if (!m_bSelectionActive)
			return;

		ShowHintLocal("Selección HALO cancelada.", "TFR HALO", 3.0);

		LogTFR("Map closed. Selection cancelled.");
		DisableHaloMapSelection(false);
	}

	//------------------------------------------------------------------------------------------------
	protected void DisableHaloMapSelection(bool closeMap)
	{
		if (m_bSelectionActive)
			LogTFR("HALO map selection disabled.");

		m_bSelectionActive = false;

		SCR_MapEntity.GetOnSelection().Remove(OnMapSelection);
		SCR_MapEntity.GetOnMapClose().Remove(OnMapClose);

		if (GetGame())
			GetGame().GetCallqueue().Remove(TFR_TryBindMapSelection);

		if (closeMap)
			TFR_ForceCloseMapUI();
	}

	//------------------------------------------------------------------------------------------------
	protected void RequestHaloJumpAtPosition(vector dropZonePosition)
	{
		if (dropZonePosition == vector.Zero)
		{
			ShowHintLocal("Punto HALO no válido.", "TFR HALO", 4.0);
			LogTFR("Request blocked: selected drop zone is vector.Zero.");
			return;
		}

		ShowHintLocal("Solicitud HALO enviada.", "TFR HALO", 3.0);

		m_bWaitingServerHaloResponse = true;

		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(TFR_CheckServerHaloResponseTimeout);
			GetGame().GetCallqueue().CallLater(
				TFR_CheckServerHaloResponseTimeout,
				TFR_SERVER_RESPONSE_TIMEOUT_MS,
				false
			);
		}

		if (Replication.IsServer())
		{
			RpcAsk_HaloJumpAtPosition(dropZonePosition);
			return;
		}

		Rpc(RpcAsk_HaloJumpAtPosition, dropZonePosition);

		LogTFR("RPC sent to server. DropZone=" + dropZonePosition.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_CheckServerHaloResponseTimeout()
	{
		if (!m_bWaitingServerHaloResponse)
			return;

		m_bWaitingServerHaloResponse = false;

		ShowHintLocal(
			"No hubo respuesta del servidor HALO.",
			"TFR HALO",
			6.0
		);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Server)]
	protected void RpcAsk_HaloJumpAtPosition(vector dropZonePosition)
	{
		if (!Replication.IsServer())
			return;

		if (dropZonePosition == vector.Zero)
		{
			Rpc(RpcDo_HaloServerResponse, "Punto HALO no válido.");
			return;
		}

		IEntity owner = GetOwner();

		if (!owner)
		{
			Rpc(RpcDo_HaloServerResponse, "HALO no disponible.");
			LogTFR("Server HALO map request blocked: owner null.");
			return;
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
		{
			Rpc(RpcDo_HaloServerResponse, "HALO no disponible.");
			LogTFR("Server HALO map request blocked: no PlayerManager.");
			return;
		}

		int playerId = playerManager.GetPlayerIdFromControlledEntity(owner);

		if (playerId <= 0)
		{
			Rpc(RpcDo_HaloServerResponse, "HALO no disponible para este jugador.");
			LogTFR("Server HALO map request blocked: invalid playerId.");
			return;
		}

		TFR_HaloJumpComponent halo = TFR_HaloJumpComponent.GetActiveHalo();

		if (!halo)
		{
			Rpc(RpcDo_HaloServerResponse, "Sistema HALO no disponible.");
			LogTFR("Server HALO map request blocked: no active TFR_HaloJumpComponent.");
			return;
		}

		vector approvedJumpPosition;
		string resultMessage;

		bool ok = halo.TFR_HaloJumpPlayerAtPositionForMap(
			playerId,
			dropZonePosition,
			approvedJumpPosition,
			resultMessage
		);

		if (!ok)
		{
			Rpc(RpcDo_HaloServerResponse, resultMessage);
			return;
		}

		Rpc(RpcDo_ClientAssistHaloTeleport, approvedJumpPosition, resultMessage);

		LogTFR("HALO map server+client assist sent. PlayerId=" + playerId.ToString() + " JumpPos=" + approvedJumpPosition.ToString());
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_HaloServerResponse(string message)
	{
		m_bWaitingServerHaloResponse = false;

		if (GetGame())
			GetGame().GetCallqueue().Remove(TFR_CheckServerHaloResponseTimeout);

		ShowHintLocal(message, "TFR HALO", 5.0);
	}

	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Owner)]
	protected void RpcDo_ClientAssistHaloTeleport(vector approvedJumpPosition, string message)
	{
		m_bWaitingServerHaloResponse = false;

		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(TFR_CheckServerHaloResponseTimeout);
			GetGame().GetCallqueue().Remove(TFR_ClientApplyHaloTeleport);
		}

		TFR_ForceCloseMapUI();

		ShowHintLocal(message, "TFR HALO", 5.0);

		TFR_ClientApplyHaloTeleport(approvedJumpPosition);

		GetGame().GetCallqueue().CallLater(TFR_ClientApplyHaloTeleport, 50, false, approvedJumpPosition);
		GetGame().GetCallqueue().CallLater(TFR_ClientApplyHaloTeleport, 150, false, approvedJumpPosition);
		GetGame().GetCallqueue().CallLater(TFR_ClientApplyHaloTeleport, 300, false, approvedJumpPosition);
		GetGame().GetCallqueue().CallLater(TFR_ClientApplyHaloTeleport, 600, false, approvedJumpPosition);
		GetGame().GetCallqueue().CallLater(TFR_ClientApplyHaloTeleport, 1000, false, approvedJumpPosition);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_ClientApplyHaloTeleport(vector approvedJumpPosition)
	{
		if (approvedJumpPosition == vector.Zero)
			return;

		PlayerController localPlayerController = GetGame().GetPlayerController();

		if (!localPlayerController)
			return;

		IEntity controlledEntity = localPlayerController.GetControlledEntity();

		if (!controlledEntity)
			return;

		vector mat[4];
		controlledEntity.GetWorldTransform(mat);
		mat[3] = approvedJumpPosition;

		controlledEntity.SetWorldTransform(mat);
		controlledEntity.SetOrigin(approvedJumpPosition);
		controlledEntity.Update();

		Physics physics = controlledEntity.GetPhysics();

		if (physics)
		{
			physics.SetVelocity(vector.Zero);
			physics.SetAngularVelocity(vector.Zero);
		}

		LogTFR("Client assist HALO teleport applied. Pos=" + approvedJumpPosition.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsLocalControlledOwner()
	{
		if (!m_Owner)
			return false;

		PlayerController localPlayerController = GetGame().GetPlayerController();

		if (!localPlayerController)
			return false;

		IEntity controlledEntity = localPlayerController.GetControlledEntity();

		if (!controlledEntity)
			return false;

		return controlledEntity == m_Owner;
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowHintLocal(string text, string title, float time)
	{
		if (text == string.Empty)
			return;

		SCR_HintManagerComponent hintMgr = SCR_HintManagerComponent.GetInstance();

		if (hintMgr)
			hintMgr.ShowCustomHint(text, title, time);
	}

	//------------------------------------------------------------------------------------------------
	protected void LogTFR(string msg)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR HALO MAP] " + msg, LogLevel.NORMAL);
	}
}
