class TFR_SurrenderControlComponentClass : ScriptComponentClass
{
};

class TFR_SurrenderControlComponent : ScriptComponent
{
	protected bool m_bPuedeRendirseACE = false;

	void EnableSurrender()
	{
		m_bPuedeRendirseACE = true;
	}

	bool CanSurrender()
	{
		return m_bPuedeRendirseACE;
	}
}
