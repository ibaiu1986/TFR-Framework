//------------------------------------------------------------------------------------------------
// TFR_OperationsRadialController.c
// TFR OPERATIONS
//
// Controller dedicado para abrir el menú radial TFR Operations.
//
// No usa SCR_RadialMenu.GlobalRadialMenu().
// Usa el menú dedicado TFR_OperationsRadialMenu.
//
// FUNCIONAMIENTO:
// - Filtra VALUE analógico.
// - Evita doble apertura por VALUE + DOWN.
// - Reintenta registrar input porque Workbench/settings pueden reconstruir bindings.
// - No fuerza cierre al ejecutar entradas.
// - No usa GetOnEntryPerformed.
// - No usa cierres retardados.
// - Modo toggle:
//   * Pulsar abre.
//   * Soltar no cierra.
//   * Pulsar otra vez cierra.
//   * Ejecutar opción con Perform Action/click/X.
//   * El cierre al ejecutar lo hace Close On Perform = true.
//
// FIX COMPAT:
// - InvokeOnInputOpen() ya no recibe parámetros.
// - No se usan const class fields para evitar errores "Broken expression" en Workbench.
//
// SERVER POLISH:
// - Los logs normales de registro/control pasan a Debug Logs.
// - Los warnings importantes siguen visibles.
// - No cambia la lógica del radial.
// - No cambia bindings.
// - No cambia toggle.
//------------------------------------------------------------------------------------------------

[BaseContainerProps()]
class TFR_OperationsRadialController : SCR_RadialMenuController
{
	[Attribute("false", UIWidgets.CheckBox, "Debug Logs")]
	protected bool m_bDebugLogs;

	protected bool m_bTFRInputRegistered;
	protected bool m_bTFRInputLocked;
	protected bool m_bTFRInputHeld;
	protected bool m_bTFRInternalOpenRequest;
	protected bool m_bTFRMenuOpen;

	protected string m_sTFRRegisteredOpenAction;

	protected int m_iTFRInputUnlockDelayMs = 300;
	protected int m_iTFRRegisterDelayMs = 250;
	protected float m_fTFRAnalogPressThreshold = 0.5;

	//------------------------------------------------------------------------------------------------
	void TFR_OperationsRadialController()
	{
		QueueRegisterTFRInputListener();
	}

	//------------------------------------------------------------------------------------------------
	void ~TFR_OperationsRadialController()
	{
		UnregisterTFRInputListener();
	}

	//------------------------------------------------------------------------------------------------
	protected void QueueRegisterTFRInputListener()
	{
		if (!GetGame())
			return;

		GetGame().GetCallqueue().Remove(RegisterTFRInputListener);
		GetGame().GetCallqueue().CallLater(RegisterTFRInputListener, m_iTFRRegisterDelayMs, false);
	}

	//------------------------------------------------------------------------------------------------
	protected void RegisterTFRInputListener()
	{
		if (m_bTFRInputRegistered)
			return;

		if (!m_RMControls)
		{
			Print("[TFR Operations Radial] Input listener not registered: RM Controls is null.", LogLevel.WARNING);
			QueueRegisterTFRInputListener();
			return;
		}

		if (m_RMControls.m_sOpenAction.IsEmpty())
		{
			Print("[TFR Operations Radial] Input listener not registered: Open Action is empty.", LogLevel.WARNING);
			QueueRegisterTFRInputListener();
			return;
		}

		InputManager inputManager = GetGame().GetInputManager();

		if (!inputManager)
		{
			Print("[TFR Operations Radial] Input listener not registered: InputManager is null.", LogLevel.WARNING);
			QueueRegisterTFRInputListener();
			return;
		}

		m_sTFRRegisteredOpenAction = m_RMControls.m_sOpenAction;

		inputManager.AddActionListener(
			m_sTFRRegisteredOpenAction,
			EActionTrigger.DOWN,
			OnTFRInputOpen
		);

		inputManager.AddActionListener(
			m_sTFRRegisteredOpenAction,
			EActionTrigger.VALUE,
			OnTFRInputValue
		);

		inputManager.AddActionListener(
			m_sTFRRegisteredOpenAction,
			EActionTrigger.UP,
			OnTFRInputRelease
		);

		m_bTFRInputRegistered = true;

		TFR_DebugLog("Input listener registered: " + m_sTFRRegisteredOpenAction);
	}

	//------------------------------------------------------------------------------------------------
	protected void ReRegisterTFRInputListener()
	{
		UnregisterTFRInputListener();
		QueueRegisterTFRInputListener();
	}

	//------------------------------------------------------------------------------------------------
	protected void UnregisterTFRInputListener()
	{
		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(RegisterTFRInputListener);
			GetGame().GetCallqueue().Remove(UnlockTFRInput);
		}

		if (m_bTFRInputRegistered)
		{
			InputManager inputManager = GetGame().GetInputManager();

			if (inputManager && !m_sTFRRegisteredOpenAction.IsEmpty())
			{
				inputManager.RemoveActionListener(
					m_sTFRRegisteredOpenAction,
					EActionTrigger.DOWN,
					OnTFRInputOpen
				);

				inputManager.RemoveActionListener(
					m_sTFRRegisteredOpenAction,
					EActionTrigger.VALUE,
					OnTFRInputValue
				);

				inputManager.RemoveActionListener(
					m_sTFRRegisteredOpenAction,
					EActionTrigger.UP,
					OnTFRInputRelease
				);
			}

			TFR_DebugLog("Input listener removed: " + m_sTFRRegisteredOpenAction);
		}

		m_sTFRRegisteredOpenAction = string.Empty;
		m_bTFRInputRegistered = false;
		m_bTFRInputLocked = false;
		m_bTFRInputHeld = false;
		m_bTFRInternalOpenRequest = false;
		m_bTFRMenuOpen = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTFRInputOpen()
	{
		if (m_bTFRInputHeld)
			return;

		m_bTFRInputHeld = true;
		TryOpenFromInput();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTFRInputValue(float value = 0.0, EActionTrigger reason = 0, string actionName = string.Empty)
	{
		if (value <= m_fTFRAnalogPressThreshold)
			return;

		if (m_bTFRInputHeld)
			return;

		m_bTFRInputHeld = true;
		TryOpenFromInput();
	}

	//------------------------------------------------------------------------------------------------
	protected void OnTFRInputRelease()
	{
		m_bTFRInputHeld = false;
		UnlockTFRInput();
	}

	//------------------------------------------------------------------------------------------------
	protected void TryOpenFromInput()
	{
		if (m_bTFRInputLocked)
			return;

		m_bTFRInputLocked = true;
		m_bTFRInternalOpenRequest = true;

		OnInputOpen();

		m_bTFRInternalOpenRequest = false;

		if (GetGame())
		{
			GetGame().GetCallqueue().Remove(UnlockTFRInput);
			GetGame().GetCallqueue().CallLater(UnlockTFRInput, m_iTFRInputUnlockDelayMs, false);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void UnlockTFRInput()
	{
		m_bTFRInputLocked = false;
	}

	//------------------------------------------------------------------------------------------------
	override void OnInputOpen()
	{
		if (!m_bTFRInternalOpenRequest)
		{
			if (m_bTFRInputLocked || m_bTFRInputHeld)
				return;

			m_bTFRInputLocked = true;

			if (GetGame())
			{
				GetGame().GetCallqueue().Remove(UnlockTFRInput);
				GetGame().GetCallqueue().CallLater(UnlockTFRInput, m_iTFRInputUnlockDelayMs, false);
			}
		}

		InvokeOnInputOpen();

		if (m_bTFRMenuOpen)
		{
			TFR_CloseMenuByToggle();
			return;
		}

		if (!m_sEnableControl)
		{
			Print("[TFR Operations Radial] Open blocked: control is disabled.", LogLevel.WARNING);
			return;
		}

		if (!HasControl())
			Control(m_Owner, m_RadialMenuToControl);

		if (!m_RadialMenu)
		{
			Print("[TFR Operations Radial] Cannot open: radial menu is null.", LogLevel.WARNING);
			return;
		}

		if (!m_RadialMenu.HasDisplay())
			m_RadialMenu.SetMenuDisplay();

		m_RadialMenu.Open();
		m_bTFRMenuOpen = true;
	}

	//------------------------------------------------------------------------------------------------
	override void Control(IEntity owner, SCR_RadialMenu radialMenu = null)
	{
		if (m_RadialMenu)
		{
			m_RadialMenu.GetOnControllerChanged().Remove(OnMenuControllerChanged);
			m_RadialMenu.GetOnOpen().Remove(TFR_OnMenuOpen);
			m_RadialMenu.GetOnClose().Remove(TFR_OnMenuClose);
		}

		m_Owner = owner;

		if (!radialMenu)
			radialMenu = TFR_OperationsRadialMenu.GetTFROperationsRadialMenu();

		m_RadialMenu = radialMenu;

		if (!m_RadialMenu)
		{
			Print("[TFR Operations Radial] Control failed: TFR operations radial menu is null.", LogLevel.WARNING);
			return;
		}

		m_RadialMenu.SetController(m_Owner, m_RMControls);

		m_RadialMenu.GetOnControllerChanged().Remove(OnMenuControllerChanged);
		m_RadialMenu.GetOnControllerChanged().Insert(OnMenuControllerChanged);

		m_RadialMenu.GetOnOpen().Remove(TFR_OnMenuOpen);
		m_RadialMenu.GetOnOpen().Insert(TFR_OnMenuOpen);

		m_RadialMenu.GetOnClose().Remove(TFR_OnMenuClose);
		m_RadialMenu.GetOnClose().Insert(TFR_OnMenuClose);

		InvokeOnTakeControl();

		ReRegisterTFRInputListener();

		TFR_DebugLog("Controller took control.");
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_OnMenuOpen(SCR_SelectionMenu menu)
	{
		m_bTFRMenuOpen = true;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_OnMenuClose(SCR_SelectionMenu menu)
	{
		m_bTFRMenuOpen = false;
		m_bTFRInputHeld = false;
		m_bTFRInputLocked = false;
		m_bTFRInternalOpenRequest = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_CloseMenuByToggle()
	{
		if (!m_RadialMenu)
		{
			m_bTFRMenuOpen = false;
			return;
		}

		m_RadialMenu.Close();

		m_bTFRMenuOpen = false;
		m_bTFRInputHeld = false;
		m_bTFRInputLocked = false;
		m_bTFRInternalOpenRequest = false;
	}

	//------------------------------------------------------------------------------------------------
	protected void TFR_DebugLog(string message)
	{
		if (!m_bDebugLogs)
			return;

		Print("[TFR Operations Radial Controller] " + message, LogLevel.NORMAL);
	}
}
