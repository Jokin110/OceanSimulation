#include "SceneManager.h"

SceneManager* SceneManager::m_Instance = nullptr;

SceneManager::SceneManager()
{
	
}

SceneManager::~SceneManager()
{
	if (m_Instance)
	{
		delete m_Instance;
		m_Instance = nullptr;
	}
}

bool SceneManager::Initialize()
{
	if (!m_Instance)
	{
		m_Instance = new SceneManager;
		return true;
	}

	return false;
}

void SceneManager::Start()
{
	
}

void SceneManager::Update()
{
	
}