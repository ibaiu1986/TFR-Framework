//------------------------------------------------------------------------------------------------
// TFR_HaloJumpComponent.c
// TFR OPERATIONS / FRAMEWORK
//
// HALO CORE MODULAR
//
// - Sistema independiente del MissionManager.
// - Puede ir en GameMode.
// - Server-side.
// - Sin EOnFrame.
// - Sin loops pesados.
// - Permite uso desde acción, radio, consola, trigger o scripts.
//
// IMPORTANTE MODULARIDAD:
// - Este archivo NO depende directamente de Parachute Framework.
// - No usa ParachuteComponent.
// - No usa ParachuteItemComponent.
// - Usa TFR_ParachuteBridge como puente interno seguro.
// - Si no hay provider externo de paracaídas, el core sigue compilando.
//
// API pública:
//   TFR_HaloJumpPlayer(playerId)
//   TFR_HaloJumpUser(user)
//   TFR_HaloJumpAllPlayers()
//   TFR_HaloJumpPlayerAtPosition(playerId, dropZonePosition)
//   TFR_HaloJumpUserAtPosition(user, dropZonePosition)
//   TFR_HaloJumpPlayerAtPositionForMap(playerId, dropZonePosition, out approvedJumpPosition, out resultMessage)
//
// Mantiene:
// - Selección HALO por mapa.
// - Server teleport.
// - Client owner assist desde TFR_HaloMapSelectionComponent.
// - Reintentos de teleport.
// - Hints.
// - APIs anteriores.
//
// Nota:
// - Las opciones Require External Parachute Provider y Auto Deploy External Parachute
//   solo tendrán efecto real cuando exista un módulo opcional que registre soporte externo.
//------------------------------------------------------------------------------------------------

class TFR_HaloJumpComponentClass : ScriptComponentClass
{
};

//------------------------------------------------------------------------------------------------
class TFR_HaloJumpComponent : ScriptComponent
{
	protected static TFR_HaloJumpComponent s_ActiveHalo;

	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	[Attribute("false", UIWidgets.CheckBox, "Require External Parachute Provider")]
	protected bool m_bRequireParachuteComponent;

	[Attribute("false", UIWidgets.CheckBox, "Require External Parachute Item")]
	protected bool m_bRequireParachuteItem;

	[Attribute("true", UIWidgets.CheckBox, "Block If Player Is In Vehicle")]
	protected bool m_bBlockIfInVehicle;

	[Attribute("true", UIWidgets.CheckBox, "Clear Velocity On Jump")]
	protected bool m_bClearVelocityOnJump;

	[Attribute("false", UIWidgets.CheckBox, "Auto Deploy External Parachute")]
	protected bool m_bAutoDeployParachute;

	[Attribute("2", UIWidgets.EditBox, "Auto Deploy Delay Seconds")]
	protected int m_iAutoDeployDelaySeconds;

	[Attribute("3", UIWidgets.EditBox, "Auto Deploy Retry Count")]
	protected int m_iAutoDeployRetryCount;

	[Attribute("1", UIWidgets.EditBox, "Auto Deploy Retry Interval Seconds")]
	protected int m_iAutoDeployRetryIntervalSeconds;

	[Attribute("900", UIWidgets.EditBox, "Jump Altitude AGL")]
	protected float m_fJumpAltitudeAGL;

	[Attribute("25", UIWidgets.EditBox, "Jump Scatter Radius")]
	protected float m_fJumpScatterRadius;

	[Attribute("true", UIWidgets.CheckBox, "Use Owner Position As Drop Zone")]
	protected bool m_bUseOwnerPositionAsDropZone;

	[Attribute("0 0 0", UIWidgets.EditBox, "Drop Zone Override Position")]
	protected vector m_vDropZoneOverridePosition;

	[Attribute("true", UIWidgets.CheckBox, "Show Hint")]
	protected bool m_bShowHint;

	[Attribute("Inserción HALO iniciada. Mantén la formación y comprueba tu equipo.", UIWidgets.EditBox, "Hint Text")]
	protected string m_sHintText;

	protected const int TFR_HALO_POSITION_INITIAL_DELAY_MS = 350;
	protected const int TFR_HALO_POSITION_RETRY_COUNT = 6;
	protected const int TFR_HALO_POSITION_RETRY_DELAY_MS = 350;

	protected IEntity m_Owner;

	//------------------------------------------------------------------------------------------------
	void TFR_HaloJumpComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		// No forzar aquí valores configurables.
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		m_Owner = owner;
		s_ActiveHalo = this;

		if (m_fJumpAltitudeAGL <= 0)
			m_fJumpAltitudeAGL = 900;

		if (m_fJumpScatterRadius < 0)
			m_fJumpScatterRadius = 0;

		if (m_iAutoDeployDelaySeconds < 0)
			m_iAutoDeployDelaySeconds = 0;

		if (m_iAutoDeployRetryCount < 0)
			m_iAutoDeployRetryCount = 0;

		if (m_iAutoDeployRetryIntervalSeconds <= 0)
			m_iAutoDeployRetryIntervalSeconds = 1;

		TFR_ParachuteBridge.SetDebugLogs(m_bDebugLogs);

		LogTFR("HALO component initialized. Owner=" + owner.ToString());
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(TFR_HaloJumpAllPlayers);
			GetGame().GetCallqueue().Remove(TFR_HaloJumpAllPlayers_Internal);
			GetGame().GetCallqueue().Remove(TFR_HaloJumpPlayer);
			GetGame().GetCallqueue().Remove(TFR_HaloJumpPlayer_Internal);
			GetGame().GetCallqueue().Remove(TFR_HaloJumpPlayerAtPosition);
			GetGame().GetCallqueue().Remove(TFR_HaloJumpPlayerAtPosition_Retry);
			GetGame().GetCallqueue().Remove(TFR_HaloJumpUserAtPosition);
			GetGame().GetCallqueue().Remove(TFR_HaloAutoDeployPlayer);
			GetGame().GetCallqueue().Remove(TFR_HaloAutoDeployPlayerRetry);
			GetGame().GetCallqueue().Remove(ApplyHaloTeleport);
		}

		if (s_ActiveHalo == this)
			s_ActiveHalo = null;

		super.OnDelete(owner);
	}

	//------------------------------------------------------------------------------------------------
	static TFR_HaloJumpComponent GetActiveHalo()
	{
		return s_ActiveHalo;
	}

	//------------------------------------------------------------------------------------------------
	void TFR_HaloJumpAllPlayers()
	{
		if (!CanRunServerLogic())
			return;

		TFR_HaloJumpAllPlayers_Internal();
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_HaloJumpAllPlayers_Internal()
	{
		if (!CanRunServerLogic())
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return;

		ref array<int> playerIds = new array<int>();
		playerManager.GetPlayers(playerIds);

		foreach (int playerId : playerIds)
		{
			TFR_HaloJumpPlayer(playerId);
		}
	}

	//------------------------------------------------------------------------------------------------
	void TFR_HaloJumpPlayer(int playerId)
	{
		if (!CanRunServerLogic())
			return;

		if (playerId <= 0)
		{
			LogTFR("HALO blocked: invalid playerId.");
			return;
		}

		TFR_HaloJumpPlayer_Internal(playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_HaloJumpPlayer_Internal(int playerId)
	{
		if (!CanRunServerLogic())
			return;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return;

		SCR_PlayerController playerController = SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));

		if (!playerController)
		{
			LogTFR("HALO blocked: no SCR_PlayerController for playerId=" + playerId.ToString());
			return;
		}

		IEntity controlledEntity = playerController.GetControlledEntity();

		if (!controlledEntity)
		{
			LogTFR("HALO blocked: no controlled entity for playerId=" + playerId.ToString());
			return;
		}

		TFR_HaloJumpUser(controlledEntity);
	}

	//------------------------------------------------------------------------------------------------
	void TFR_HaloJumpUser(IEntity user)
	{
		if (!CanRunServerLogic())
			return;

		if (!user)
		{
			LogTFR("HALO blocked: user null.");
			return;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);

		if (!character)
		{
			LogTFR("HALO blocked: user is not SCR_ChimeraCharacter.");
			return;
		}

		int playerId = GetPlayerIdFromCharacter(character);

		if (playerId <= 0)
		{
			LogTFR("HALO blocked: no valid playerId for user.");
			return;
		}

		SCR_PlayerController playerController = GetPlayerController(playerId);

		if (!playerController)
		{
			LogTFR("HALO blocked: no player controller.");
			return;
		}

		if (!CanPlayerHaloJump(playerController, character))
			return;

		vector jumpPos = BuildJumpPosition();

		if (jumpPos == vector.Zero)
		{
			LogTFR("HALO blocked: invalid jump position.");
			return;
		}

		TeleportCharacterToHalo(character, jumpPos);

		if (m_bShowHint)
			ShowHintSafe(m_sHintText, "TFR HALO", 6.0);

		if (m_bAutoDeployParachute)
			ScheduleAutoDeploy(playerId);

		LogTFR("HALO jump executed. PlayerId=" + playerId.ToString() + " Pos=" + jumpPos.ToString());
	}

	//------------------------------------------------------------------------------------------------
	void TFR_HaloJumpInstigator(IEntity instigator)
	{
		TFR_HaloJumpUser(instigator);
	}

	//------------------------------------------------------------------------------------------------
	void TFR_HaloJumpPlayerAtPosition(int playerId, vector dropZonePosition)
	{
		if (!CanRunServerLogic())
			return;

		if (playerId <= 0)
		{
			LogTFR("HALO at position blocked: invalid playerId.");
			return;
		}

		if (!IsValidDropZonePosition(dropZonePosition))
		{
			LogTFR("HALO at position blocked: invalid drop zone position.");
			return;
		}

		GetGame().GetCallqueue().CallLater(
			TFR_HaloJumpPlayerAtPosition_Retry,
			TFR_HALO_POSITION_INITIAL_DELAY_MS,
			false,
			playerId,
			dropZonePosition,
			0
		);
	}

	//------------------------------------------------------------------------------------------------
	bool TFR_HaloJumpPlayerAtPositionForMap(int playerId, vector dropZonePosition, out vector approvedJumpPosition, out string resultMessage)
	{
		approvedJumpPosition = vector.Zero;
		resultMessage = string.Empty;

		if (!CanRunServerLogic())
		{
			resultMessage = "HALO no disponible.";
			return false;
		}

		if (playerId <= 0)
		{
			resultMessage = "Jugador HALO no válido.";
			return false;
		}

		if (!IsValidDropZonePosition(dropZonePosition))
		{
			resultMessage = "Punto HALO no válido.";
			return false;
		}

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
		{
			resultMessage = "Sistema HALO no disponible.";
			return false;
		}

		IEntity controlledEntity = playerManager.GetPlayerControlledEntity(playerId);

		if (!controlledEntity)
		{
			SCR_PlayerController fallbackController = GetPlayerController(playerId);

			if (fallbackController)
				controlledEntity = fallbackController.GetControlledEntity();
		}

		if (!controlledEntity)
		{
			resultMessage = "Jugador HALO no encontrado.";
			return false;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(controlledEntity);

		if (!character)
		{
			resultMessage = "Entidad HALO no válida.";
			return false;
		}

		if (character.IsDeleted())
		{
			resultMessage = "Jugador HALO no válido.";
			return false;
		}

		SCR_PlayerController playerController = GetPlayerController(playerId);

		if (!playerController)
		{
			resultMessage = "Controlador HALO no encontrado.";
			return false;
		}

		if (!CanPlayerHaloJump(playerController, character))
		{
			resultMessage = "No cumples los requisitos para inserción HALO.";
			return false;
		}

		vector jumpPos = BuildJumpPositionFromDropZone(dropZonePosition);

		if (jumpPos == vector.Zero)
		{
			resultMessage = "Posición HALO no válida.";
			return false;
		}

		approvedJumpPosition = jumpPos;

		TeleportCharacterToHalo(character, jumpPos);

		if (m_bShowHint)
			ShowHintSafe(m_sHintText, "TFR HALO", 6.0);

		if (m_bAutoDeployParachute)
			ScheduleAutoDeploy(playerId);

		resultMessage = "Inserción HALO iniciada.";

		LogTFR("HALO map jump executed. PlayerId=" + playerId.ToString() + " DropZone=" + dropZonePosition.ToString() + " JumpPos=" + jumpPos.ToString());

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_HaloJumpPlayerAtPosition_Retry(int playerId, vector dropZonePosition, int attempt)
	{
		if (!CanRunServerLogic())
			return;

		string failReason;
		bool success = TFR_TryExecuteHaloJumpPlayerAtPosition(playerId, dropZonePosition, failReason);

		if (success)
		{
			LogTFR("HALO at position retry succeeded. PlayerId=" + playerId.ToString() + " Attempt=" + attempt.ToString());
			return;
		}

		if (attempt >= TFR_HALO_POSITION_RETRY_COUNT)
		{
			LogTFR("HALO at position finally failed. PlayerId=" + playerId.ToString() + " Reason=" + failReason);
			return;
		}

		GetGame().GetCallqueue().CallLater(
			TFR_HaloJumpPlayerAtPosition_Retry,
			TFR_HALO_POSITION_RETRY_DELAY_MS,
			false,
			playerId,
			dropZonePosition,
			attempt + 1
		);
	}

	//------------------------------------------------------------------------------------------------
	protected bool TFR_TryExecuteHaloJumpPlayerAtPosition(int playerId, vector dropZonePosition, out string failReason)
	{
		failReason = string.Empty;

		vector approvedJumpPosition;
		string resultMessage;

		bool ok = TFR_HaloJumpPlayerAtPositionForMap(
			playerId,
			dropZonePosition,
			approvedJumpPosition,
			resultMessage
		);

		if (!ok)
		{
			failReason = resultMessage;
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	void TFR_HaloJumpUserAtPosition(IEntity user, vector dropZonePosition)
	{
		if (!CanRunServerLogic())
			return;

		if (!user)
		{
			LogTFR("HALO at position blocked: user null.");
			return;
		}

		if (!IsValidDropZonePosition(dropZonePosition))
		{
			LogTFR("HALO at position blocked: invalid drop zone position.");
			return;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(user);

		if (!character)
		{
			LogTFR("HALO at position blocked: user is not SCR_ChimeraCharacter.");
			return;
		}

		int playerId = GetPlayerIdFromCharacter(character);

		if (playerId <= 0)
		{
			LogTFR("HALO at position blocked: no valid playerId for user.");
			return;
		}

		SCR_PlayerController playerController = GetPlayerController(playerId);

		if (!playerController)
		{
			LogTFR("HALO at position blocked: no player controller.");
			return;
		}

		if (!CanPlayerHaloJump(playerController, character))
			return;

		vector jumpPos = BuildJumpPositionFromDropZone(dropZonePosition);

		if (jumpPos == vector.Zero)
		{
			LogTFR("HALO at position blocked: invalid jump position.");
			return;
		}

		TeleportCharacterToHalo(character, jumpPos);

		if (m_bShowHint)
			ShowHintSafe(m_sHintText, "TFR HALO", 6.0);

		if (m_bAutoDeployParachute)
			ScheduleAutoDeploy(playerId);

		LogTFR("HALO jump at position executed. PlayerId=" + playerId.ToString() + " DropZone=" + dropZonePosition.ToString() + " Pos=" + jumpPos.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected void ScheduleAutoDeploy(int playerId)
	{
		if (playerId <= 0)
			return;

		int delayMs = Math.Max(m_iAutoDeployDelaySeconds, 0) * 1000;
		GetGame().GetCallqueue().CallLater(TFR_HaloAutoDeployPlayer, delayMs, false, playerId);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_HaloAutoDeployPlayer(int playerId)
	{
		TFR_HaloAutoDeployPlayerRetry(playerId, 0);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_HaloAutoDeployPlayerRetry(int playerId, int attempt)
	{
		if (!CanRunServerLogic())
			return;

		if (playerId <= 0)
			return;

		SCR_PlayerController playerController = GetPlayerController(playerId);

		if (!playerController)
		{
			LogTFR("Auto deploy blocked: no player controller.");
			return;
		}

		IEntity controlledEntity = playerController.GetControlledEntity();

		if (!controlledEntity)
		{
			LogTFR("Auto deploy blocked: no controlled entity.");
			return;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(controlledEntity);

		if (!character)
		{
			LogTFR("Auto deploy blocked: controlled entity is not character.");
			return;
		}

		if (character.IsDeleted())
			return;

		if (character.IsInVehicle())
		{
			LogTFR("Auto deploy ignored: character already in vehicle/parachute.");
			return;
		}

		bool deployRequested = TFR_ParachuteBridge.RequestAutoDeploy(playerController, character);

		if (!deployRequested)
		{
			LogTFR("Auto deploy not executed: no external parachute provider or provider rejected request.");

			if (attempt < m_iAutoDeployRetryCount)
			{
				int retryDelayMs = Math.Max(m_iAutoDeployRetryIntervalSeconds, 1) * 1000;
				GetGame().GetCallqueue().CallLater(TFR_HaloAutoDeployPlayerRetry, retryDelayMs, false, playerId, attempt + 1);
			}

			return;
		}

		LogTFR("Auto deploy requested through provider=" + TFR_ParachuteBridge.GetProviderName() + " PlayerId=" + playerId.ToString() + " Attempt=" + attempt.ToString());

		if (attempt < m_iAutoDeployRetryCount)
		{
			int retryDelayMs = Math.Max(m_iAutoDeployRetryIntervalSeconds, 1) * 1000;
			GetGame().GetCallqueue().CallLater(TFR_HaloAutoDeployPlayerRetry, retryDelayMs, false, playerId, attempt + 1);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanPlayerHaloJump(SCR_PlayerController playerController, SCR_ChimeraCharacter character)
	{
		if (!playerController)
			return false;

		if (!character)
			return false;

		if (character.IsDeleted())
			return false;

		if (m_bBlockIfInVehicle && character.IsInVehicle())
		{
			LogTFR("HALO blocked: player is in vehicle.");
			return false;
		}

		if (!TFR_ParachuteBridge.CanHaloJump(playerController, character, m_bRequireParachuteComponent, m_bRequireParachuteItem))
		{
			LogTFR("HALO blocked: external parachute requirements not met or provider unavailable.");
			return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected vector BuildJumpPosition()
	{
		vector basePos = GetDropZoneBasePosition();

		if (!IsValidDropZonePosition(basePos))
		{
			LogTFR("HALO blocked: base drop zone position invalid.");
			return vector.Zero;
		}

		return BuildJumpPositionFromDropZone(basePos);
	}

	//------------------------------------------------------------------------------------------------
	protected vector BuildJumpPositionFromDropZone(vector dropZonePosition)
	{
		if (!IsValidDropZonePosition(dropZonePosition))
			return vector.Zero;

		vector pos = dropZonePosition;

		if (m_fJumpScatterRadius > 0)
			pos = GetRandomOffsetPosition(dropZonePosition, 0, m_fJumpScatterRadius);

		float terrainY = GetTerrainY(pos);
		pos[1] = terrainY + m_fJumpAltitudeAGL;

		return pos;
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetDropZoneBasePosition()
	{
		vector spawnPointPos;

		if (TFR_HaloSpawnPointComponent.GetBestSpawnPosition(spawnPointPos))
		{
			LogTFR("Using HALO spawn point as drop zone. Pos=" + spawnPointPos.ToString());
			return spawnPointPos;
		}

		if (m_bUseOwnerPositionAsDropZone)
		{
			if (m_Owner)
			{
				LogTFR("Using owner position as drop zone.");
				return m_Owner.GetOrigin();
			}

			IEntity owner = GetOwner();

			if (owner)
			{
				LogTFR("Using GetOwner position as drop zone.");
				return owner.GetOrigin();
			}
		}

		LogTFR("Using override position as drop zone. Pos=" + m_vDropZoneOverridePosition.ToString());
		return m_vDropZoneOverridePosition;
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsValidDropZonePosition(vector pos)
	{
		if (pos == vector.Zero)
			return false;

		if (!GetGame())
			return false;

		if (!GetGame().GetWorld())
			return false;

		float terrainY = GetTerrainY(pos);

		if (terrainY < -10)
			return false;

		return true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TeleportCharacterToHalo(SCR_ChimeraCharacter character, vector jumpPos)
	{
		if (!character)
			return;

		ApplyHaloTeleport(character, jumpPos);

		GetGame().GetCallqueue().CallLater(ApplyHaloTeleport, 50, false, character, jumpPos);
		GetGame().GetCallqueue().CallLater(ApplyHaloTeleport, 150, false, character, jumpPos);
		GetGame().GetCallqueue().CallLater(ApplyHaloTeleport, 300, false, character, jumpPos);
		GetGame().GetCallqueue().CallLater(ApplyHaloTeleport, 600, false, character, jumpPos);
		GetGame().GetCallqueue().CallLater(ApplyHaloTeleport, 1000, false, character, jumpPos);
		GetGame().GetCallqueue().CallLater(ApplyHaloTeleport, 1500, false, character, jumpPos);
		GetGame().GetCallqueue().CallLater(ApplyHaloTeleport, 2500, false, character, jumpPos);
		GetGame().GetCallqueue().CallLater(ApplyHaloTeleport, 3500, false, character, jumpPos);
	}

	//------------------------------------------------------------------------------------------------
	protected void ApplyHaloTeleport(SCR_ChimeraCharacter character, vector jumpPos)
	{
		if (!character)
			return;

		if (character.IsDeleted())
			return;

		vector mat[4];
		character.GetWorldTransform(mat);
		mat[3] = jumpPos;

		character.SetWorldTransform(mat);
		character.SetOrigin(jumpPos);
		character.Update();

		if (m_bClearVelocityOnJump)
		{
			Physics physics = character.GetPhysics();

			if (physics)
			{
				physics.SetVelocity(vector.Zero);
				physics.SetAngularVelocity(vector.Zero);
			}
		}

		LogTFR("HALO teleport applied. Pos=" + jumpPos.ToString());
	}

	//------------------------------------------------------------------------------------------------
	protected vector GetRandomOffsetPosition(vector basePos, float minRadius, float maxRadius)
	{
		if (maxRadius < minRadius)
			maxRadius = minRadius + 1;

		float angle = Math.RandomFloat01() * Math.PI2;
		float radius = Math.RandomFloat(minRadius, maxRadius);

		vector pos = basePos;
		pos[0] = pos[0] + Math.Cos(angle) * radius;
		pos[2] = pos[2] + Math.Sin(angle) * radius;

		pos[1] = GetTerrainY(pos);

		return pos;
	}

	//------------------------------------------------------------------------------------------------
	protected float GetTerrainY(vector pos)
	{
		if (!GetGame())
			return pos[1];

		if (!GetGame().GetWorld())
			return pos[1];

		return SCR_TerrainHelper.GetTerrainY(pos, null, true);
	}

	//------------------------------------------------------------------------------------------------
	protected int GetPlayerIdFromCharacter(IEntity character)
	{
		if (!character)
			return 0;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return 0;

		return playerManager.GetPlayerIdFromControlledEntity(character);
	}

	//------------------------------------------------------------------------------------------------
	protected SCR_PlayerController GetPlayerController(int playerId)
	{
		if (playerId <= 0)
			return null;

		PlayerManager playerManager = GetGame().GetPlayerManager();

		if (!playerManager)
			return null;

		return SCR_PlayerController.Cast(playerManager.GetPlayerController(playerId));
	}

	//------------------------------------------------------------------------------------------------
	protected bool IsServer()
	{
		if (!GetGame())
			return false;

		return Replication.IsServer();
	}

	//------------------------------------------------------------------------------------------------
	protected bool CanRunServerLogic()
	{
		if (!m_bEnabled)
			return false;

		return IsServer();
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowHintSafe(string text, string title, float time)
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

		Print("[TFR HALO] " + msg, LogLevel.NORMAL);
	}
}
