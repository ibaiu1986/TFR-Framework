class TFR_CivilDataComponentClass : ScriptComponentClass
{
};

class TFR_CivilDataComponent : ScriptComponent
{
	[Attribute("true", UIWidgets.CheckBox, "Enabled")]
	bool m_bEnabled;

	// Mantengo el nombre antiguo porque TFR_Action_Arrestar lo usa.
	// Valores aceptados: "si", "sí", "true", "1".
	[Attribute("no", UIWidgets.EditBox, "Arrest Order")]
	string m_sOrdenArresto;

	// Mantengo el nombre antiguo porque TFR_Action_Arrestar lo usa.
	[Attribute("true", UIWidgets.CheckBox, "Can Surrender")]
	bool m_bPuedeRendirse;

	// Variable interna. La usa TFR_Action_Identificar y TFR_Action_Arrestar.
	// No lleva Attribute para que no salga en Workbench.
	bool m_bIdentificado = false;

	bool IsEnabled()
	{
		return m_bEnabled;
	}

	bool HasArrestOrder()
	{
		if (!m_bEnabled)
			return false;

		string orden = m_sOrdenArresto;
		orden.ToLower();

		if (orden == "si")
			return true;

		if (orden == "sí")
			return true;

		if (orden == "true")
			return true;

		if (orden == "1")
			return true;

		return false;
	}

	bool CanSurrender()
	{
		if (!m_bEnabled)
			return false;

		return m_bPuedeRendirse;
	}

	bool IsIdentified()
	{
		return m_bIdentificado;
	}

	void SetIdentified(bool state)
	{
		m_bIdentificado = state;
	}
}
