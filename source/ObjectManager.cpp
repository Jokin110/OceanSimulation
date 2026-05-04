#include "ObjectManager.h"
#include "Object.h"

ObjectManager* ObjectManager::m_Instance = nullptr;

ObjectManager::ObjectManager()
{
	m_Objects.clear();
}

ObjectManager::~ObjectManager()
{
	for (int i = 0; i < m_Objects.size(); i++)
	{
		delete m_Objects[i];
		m_Objects[i] = nullptr;
	} 

	m_Objects.clear();

	if (m_Instance)
	{
		m_Instance = nullptr;
	}
}

bool ObjectManager::Initialize()
{
	if (!m_Instance)
	{
		m_Instance = new ObjectManager;
		return true;
	}

	return false;
}

bool ObjectManager::InitializeObjects()
{
	for (int i = 0; i < m_Objects.size(); i++)
	{
		if (!m_Objects[i]->Initialize())
			return false;
	}

	return true;
}

void ObjectManager::AddObjectToList(IObject* object)
{
	m_Objects.push_back(object);
}

void ObjectManager::Start()
{
	for (int i = 0; i < m_Objects.size(); i++)
	{
		m_Objects[i]->Start();
	}
}

void ObjectManager::Update()
{
	for (int i = 0; i < m_Objects.size(); i++)
	{
		m_Objects[i]->Update();
	}
}