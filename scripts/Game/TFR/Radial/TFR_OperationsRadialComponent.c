//------------------------------------------------------------------------------------------------
// TFR_OperationsRadialComponent.c
// TFR OPERATIONS
//
// Componente cliente que controla y rellena el menú radial dedicado de TFR Operations.
//
// VA EN EL PERSONAJE / PREFAB DE JUGADOR.
//
// No ejecuta lógica server sensible directamente.
// - HALO abre selección local por mapa.
// - Admin misión/persistencia van por TFR_OperationsRadialNetworkComponent en PlayerController.
//
// DEDICATED FIX:
// - En servidor dedicado el personaje puede inicializar antes de que el GameMode registre
//   TFR_OperationsRadialMenu en el cliente.
// - Reintenta tomar control hasta que el menú dedicado exista.
// - Imprime BOOT antes de cortar por System.IsConsoleApp() para comprobar desde log servidor
//   si el prefab real del jugador lleva este componente.
//
// COMPAT FIX:
// - GetOnControllerChanged() usa ScriptInvoker_RadialMenuEventMethod.
// - La firma compatible es OnControllerChanged(SCR_RadialMenu radialMenu).
// - No se elimina funcionalidad.
//------------------------------------------------------------------------------------------------

[ComponentEditorProps(category: "TFR Operations/Radial", description: "Controls the TFR Operations radial menu.")]
class TFR_OperationsRadialComponentClass : ScriptComponentClass
{
}

//------------------------------------------------------------------------------------------------
class TFR_OperationsRadialComponent : ScriptComponent
{
	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	protected bool m_bEnabled;

	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebug;

	[Attribute("TFR Operations", UIWidgets.EditBox, "Hint Title")]
	protected string m_sHintTitle;

	[Attribute("4", UIWidgets.EditBox, "Hint Duration Seconds")]
	protected float m_fHintDuration;

	[Attribute("true", UIWidgets.CheckBox, "Show HALO Entry")]
	protected bool m_bShowHaloEntry;

	[Attribute("true", UIWidgets.CheckBox, "Show Mission Admin Entries")]
	protected bool m_bShowMissionAdminEntries;

	[Attribute("true", UIWidgets.CheckBox, "Show Persistence Entries")]
	protected bool m_bShowPersistenceEntries;

	[Attribute(desc: "Dedicated TFR Operations radial controller.")]
	protected ref TFR_OperationsRadialController m_MenuController;

	protected IEntity m_Owner;
	protected SCR_RadialMenu m_RadialMenu;

	protected int m_iControlRetryCount;

	protected int m_iTFRControlRetryDelayMs = 500;
	protected int m_iTFRControlRetryMax = 40;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		Print("[TFR Operations Radial BOOT SERVERCHECK] Character radial component reached. IsConsoleApp=" + System.IsConsoleApp().ToString() + " Owner=" + owner.ToString(), LogLevel.NORMAL);

		m_Owner = owner;
		m_iControlRetryCount = 0;

		if (!m_bEnabled)
			return;

		if (System.IsConsoleApp())
			return;

		if (!GetGame())
			return;

		if (!GetGame().InPlayMode())
			return;

		if (m_sHintTitle == string.Empty)
			m_sHintTitle = "TFR Operations";

		if (m_fHintDuration <= 0)
			m_fHintDuration = 4.0;

		if (!m_MenuController)
		{
			Print("[TFR Operations Radial] Component: menu controller is not configured.", LogLevel.WARNING);
			return;
		}

		SetEventMask(owner, EntityEvent.INIT);

		m_MenuController.GetOnTakeControl().Remove(OnControllerTakeControl);
		m_MenuController.GetOnTakeControl().Insert(OnControllerTakeControl);

		m_MenuController.GetOnControllerChanged().Remove(OnControllerChanged);
		m_MenuController.GetOnControllerChanged().Insert(OnControllerChanged);

		TFR_DebugLog("Operations radial component initialized.");
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (!m_bEnabled)
			return;

		if (!m_MenuController)
			return;

		TFR_RequestControllerControlDelayed();
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_RequestControllerControlDelayed()
	{
		if (!m_bEnabled)
			return;

		if (!m_MenuController)
			return;

		if (!GetGame())
			return;

		GetGame().GetCallqueue().Remove(TFR_TryTakeRadialControl);
		GetGame().GetCallqueue().CallLater(TFR_TryTakeRadialControl, 100, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_TryTakeRadialControl()
	{
		if (!m_bEnabled)
			return;

		if (!m_MenuController)
			return;

		if (!m_Owner)
			return;

		if (System.IsConsoleApp())
			return;

		TFR_OperationsRadialMenu tfrMenu = TFR_OperationsRadialMenu.GetTFROperationsRadialMenu();

		if (!tfrMenu)
		{
			if (m_iControlRetryCount >= m_iTFRControlRetryMax)
			{
				Print("[TFR Operations Radial] Control failed: dedicated radial menu was never registered on client.", LogLevel.WARNING);
				return;
			}

			m_iControlRetryCount++;

			TFR_DebugLog("Waiting for dedicated radial menu. Retry=" + m_iControlRetryCount.ToString());

			GetGame().GetCallqueue().Remove(TFR_TryTakeRadialControl);
			GetGame().GetCallqueue().CallLater(TFR_TryTakeRadialControl, m_iTFRControlRetryDelayMs, false);
			return;
		}

		m_MenuController.Control(m_Owner, tfrMenu);

		TFR_DebugLog("Operations radial controller control requested.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControllerTakeControl(SCR_RadialMenuController controller)
	{
		if (!controller)
			return;

		SCR_RadialMenu radialMenu = controller.GetRadialMenu();

		if (!radialMenu)
		{
			Print("[TFR Operations Radial] Controller took control but radial menu is null.", LogLevel.WARNING);
			return;
		}

		if (m_RadialMenu && m_RadialMenu != radialMenu)
			UnregisterRadialMenu(m_RadialMenu);

		m_RadialMenu = radialMenu;
		RegisterRadialMenu(m_RadialMenu);

		TFR_DebugLog("Operations radial controller took control.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnControllerChanged(SCR_RadialMenu radialMenu)
	{
		if (!radialMenu)
			return;

		if (m_RadialMenu && m_RadialMenu != radialMenu)
			UnregisterRadialMenu(m_RadialMenu);

		m_RadialMenu = radialMenu;
		RegisterRadialMenu(m_RadialMenu);

		TFR_DebugLog("Operations radial controller changed.");
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterRadialMenu(SCR_RadialMenu radialMenu)
	{
		if (!radialMenu)
			return;

		radialMenu.GetOnBeforeOpen().Remove(OnBeforeMenuOpen);
		radialMenu.GetOnOpen().Remove(OnMenuOpen);
		radialMenu.GetOnClose().Remove(OnMenuClose);

		radialMenu.GetOnBeforeOpen().Insert(OnBeforeMenuOpen);
		radialMenu.GetOnOpen().Insert(OnMenuOpen);
		radialMenu.GetOnClose().Insert(OnMenuClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterRadialMenu(SCR_RadialMenu radialMenu)
	{
		if (!radialMenu)
			return;

		radialMenu.GetOnBeforeOpen().Remove(OnBeforeMenuOpen);
		radialMenu.GetOnOpen().Remove(OnMenuOpen);
		radialMenu.GetOnClose().Remove(OnMenuClose);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnBeforeMenuOpen(SCR_SelectionMenu menu)
	{
		SCR_RadialMenu radialMenu = SCR_RadialMenu.Cast(menu);

		if (!radialMenu)
			return;

		radialMenu.ClearEntries();
		CreateEntries(radialMenu);

		TFR_DebugLog("Operations radial entries rebuilt before open.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuOpen(SCR_SelectionMenu menu)
	{
		TFR_DebugLog("Operations radial opened.");
	}

	//------------------------------------------------------------------------------------------------
	protected void OnMenuClose(SCR_SelectionMenu menu)
	{
		TFR_DebugLog("Operations radial closed.");
	}

	//------------------------------------------------------------------------------------------------
	protected void CreateEntries(notnull SCR_RadialMenu radialMenu)
	{
		array<ref SCR_SelectionMenuEntry> entries = {};

		if (m_bShowHaloEntry)
		{
			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.HALO_MAP,
				"HALO",
				"Solicitar inserción HALO por mapa",
				m_sHintTitle,
				m_fHintDuration
			));
		}

		if (m_bShowMissionAdminEntries)
		{
			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.MISSION_PRINT_STATE,
				"Diagnóstico misión",
				"Imprime el estado de misión TFR en servidor",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.MISSION_FORCE_NEXT_INTEL,
				"Forzar intel",
				"Avanza la cadena de inteligencia",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.MISSION_COMPLETE,
				"Completar misión",
				"Marca la misión activa como completada",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.MISSION_FAIL,
				"Fallar misión",
				"Marca la misión activa como fallida",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.MISSION_RESTART_INTEL,
				"Reiniciar intel",
				"Reinicia la cadena de inteligencia TFR",
				m_sHintTitle,
				m_fHintDuration
			));
		}

		if (m_bShowPersistenceEntries)
		{
			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.PERSISTENCE_SAVE_OPERATION,
				"Guardar operación",
				"Guarda el estado lógico de operación",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.PERSISTENCE_LOAD_OPERATION,
				"Cargar operación",
				"Carga el estado lógico de operación",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.PERSISTENCE_RESET_OPERATION,
				"Reset operación",
				"Borra persistencia de operación",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.PERSISTENCE_RESET_PLAYER_INVENTORY,
				"Reset inventario",
				"Borra persistencia de inventario de jugadores",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.PERSISTENCE_RESET_VEHICLE_CARGO,
				"Reset vehículos",
				"Borra persistencia de carga de vehículos",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.PERSISTENCE_RESET_ALL,
				"Reset total",
				"Borra toda la persistencia TFR",
				m_sHintTitle,
				m_fHintDuration
			));

			entries.Insert(new TFR_OperationsRadialEntry(
				TFR_EOperationsRadialEntryType.PERSISTENCE_PRINT_STATE,
				"Diagnóstico persist.",
				"Imprime estado de persistencia TFR en servidor",
				m_sHintTitle,
				m_fHintDuration
			));
		}

		radialMenu.AddEntries(entries, true);
		radialMenu.UpdateEntries();
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_DebugLog(string message)
	{
		if (!m_bDebug)
			return;

		Print("[TFR Operations Radial] " + message, LogLevel.NORMAL);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (GetGame())
			GetGame().GetCallqueue().Remove(TFR_TryTakeRadialControl);

		if (m_MenuController)
		{
			m_MenuController.GetOnTakeControl().Remove(OnControllerTakeControl);
			m_MenuController.GetOnControllerChanged().Remove(OnControllerChanged);
			m_MenuController.StopControl(false);
		}

		UnregisterRadialMenu(m_RadialMenu);

		m_RadialMenu = null;
		m_Owner = null;

		ClearEventMask(owner, EntityEvent.INIT);

		super.OnDelete(owner);
	}
}
