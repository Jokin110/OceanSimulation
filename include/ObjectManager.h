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

