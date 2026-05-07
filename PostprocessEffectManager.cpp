#include "PostprocessEffectManager.h"

PostprocessEffectManager::PostprocessEffectManager()
{
	m_PostprocessEffects.clear();
}

PostprocessEffectManager::~PostprocessEffectManager()
{
	for (int i = 0; i < m_PostprocessEffects.size(); i++)
	{
		delete m_PostprocessEffects[i];
		m_PostprocessEffects[i] = nullptr;
	}

	m_PostprocessEffects.clear();

	if (m_Instance)
	{
		m_Instance = nullptr;
	}
}

bool PostprocessEffectManager::Initialize()
{
	if (!m_Instance)
	{
		m_Instance = new PostprocessEffectManager;
		return true;
	}

	return false;
}

bool PostprocessEffectManager::InitializeEffects()
{
	for (int i = 0; i < m_PostprocessEffects.size(); i++)
	{
		if (!m_PostprocessEffects[i]->Initialize())
			return false;
	}

	return true;
}

void PostprocessEffectManager::AddEffectToList(PostprocessEffect* effect)
{
	m_PostprocessEffects.push_back(effect);
}

void PostprocessEffectManager::Start()
{
	for (int i = 0; i < m_PostprocessEffects.size(); i++)
	{
		m_PostprocessEffects[i]->Start();
	}
}

void PostprocessEffectManager::Update()
{
	for (int i = 0; i < m_PostprocessEffects.size(); i++)
	{
		m_PostprocessEffects[i]->Update();
	}
}