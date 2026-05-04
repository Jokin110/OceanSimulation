#pragma once

#include<vector>

using namespace std;

class IObject;

class ObjectManager
{
public:
    ObjectManager();
    ~ObjectManager();

    static ObjectManager& GetInstance()
    {
        return *m_Instance;
    }

    static void DestroyInstance()
    {
        if (m_Instance)
        {
            delete m_Instance;
            m_Instance = nullptr;
        }
    }

    void AddObjectToList(IObject* object);

	vector<IObject*>& GetObjectList() { return m_Objects; }

    static bool Initialize();
	bool InitializeObjects();
    void Start();
    void Update();

private:
    static ObjectManager* m_Instance;

	vector<IObject*> m_Objects;
};

