#include "TilemapEditorManager.h"

#include "../Systems/SceneManager.h"

namespace Uma_Engine
{
    void tilemapEditorManager::OpenEditor(Entity entity)
    {
        currEntity = entity;
        currTilemap = &pCoordinator->GetComponent<Uma_ECS::Tilemap>(entity);


    }

    void tilemapEditorManager::CloseEditor()
    {
    }

    void tilemapEditorManager::IsEditing() const
    {
    }

    void tilemapEditorManager::Init()
    {
        pGraphics = pSystemManager->GetSystem<Graphics>();
        pResourcesManager = pSystemManager->GetSystem<ResourcesManager>();

        auto sceneManager = pSystemManager->GetSystem<SceneManager>();
        if (sceneManager)
        {
            pCoordinator = &sceneManager->GetActiveScene()->GetCoordinator();
        }
    }

    void tilemapEditorManager::Update(float dt)
    {
    }

    void tilemapEditorManager::Shutdown()
    {
    }
}