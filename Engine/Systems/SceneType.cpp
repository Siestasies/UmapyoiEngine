#include "SceneType.h"
#include "Core/GameSerializer.h"
#include <future>

namespace Uma_Engine
{
	Scene::Scene(const std::string& name, const std::string& filepath, SystemManager* sm) :
		m_Name(name), m_FilePath(filepath), m_SystemManager(sm)
	{
	}

	Scene::~Scene()
	{
		if (m_State == SceneState::SCENE_RUNNING)
			OnUnload();
	}

	void Scene::OnLoad()
	{
		if (m_State != SceneState::SCENE_UNLOADED)
			return;

		m_State = SceneState::SCENE_LOADING;
		LoadInternal();
		m_State = SceneState::SCENE_RUNNING;
		m_LoadProgress = 1.f;
	}

	void Scene::OnLoadAsync()
	{
		if (m_State != SceneState::Unloaded)
			return;

		m_State = SceneState::Loading;
		m_LoadProgress = 0.0f;

		// Launch async task
		m_LoadFuture = std::async(std::launch::async, [this]() {
			LoadInternal();
		});

		pSystemManager
	}

	void Scene::LoadInternal()
	{
		// HANDLERS TO SYSTEMS
		m_HybridInputSystem = m_SystemManager->GetSystem<HybridInputSystem>();
		m_Graphics = m_SystemManager->GetSystem<Graphics>();
		m_Sound = m_SystemManager->GetSystem<Sound>();
		m_EventSystem = m_SystemManager->GetSystem<EventSystem>();
		m_ResourcesManager = m_SystemManager->GetSystem<ResourcesManager>();

		// Initialize coordinator with all component types
		m_Coordinator.Init(m_EventSystem);
		// register components
		m_LoadProgress = 0.2f;
		m_Coordinator.RegisterComponent<Uma_ECS::Transform>();
		m_Coordinator.RegisterComponent<Uma_ECS::RigidBody>();
		m_Coordinator.RegisterComponent<Uma_ECS::Collider>();
		m_Coordinator.RegisterComponent<Uma_ECS::Sprite>();
		m_Coordinator.RegisterComponent<Uma_ECS::Camera>();
		m_Coordinator.RegisterComponent<Uma_ECS::Player>();
		m_Coordinator.RegisterComponent<Uma_ECS::Enemy>();

		// Player controller
		playerController = gCoordinator.RegisterSystem<PlayerControllerSystem>();
		{
			Signature sign;
			sign.set(gCoordinator.GetComponentType<RigidBody>());
			sign.set(gCoordinator.GetComponentType<Transform>());
			sign.set(gCoordinator.GetComponentType<Player>());
			gCoordinator.SetSystemSignature<PlayerControllerSystem>(sign);
		}
		playerController->Init(pEventSystem, pHybridInputSystem, &gCoordinator);

		// Physics System
		physicsSystem = gCoordinator.RegisterSystem<PhysicsSystem>();
		{
			Signature sign;
			sign.set(gCoordinator.GetComponentType<RigidBody>());
			sign.set(gCoordinator.GetComponentType<Transform>());
			gCoordinator.SetSystemSignature<PhysicsSystem>(sign);
		}
		physicsSystem->Init(&gCoordinator);

		// Collision System
		collisionSystem = gCoordinator.RegisterSystem<CollisionSystem>();
		{
			Signature sign;
			sign.set(gCoordinator.GetComponentType<RigidBody>());
			sign.set(gCoordinator.GetComponentType<Transform>());
			sign.set(gCoordinator.GetComponentType<Collider>());
			gCoordinator.SetSystemSignature<CollisionSystem>(sign);
		}
		collisionSystem->Init(&gCoordinator);

		// Rendering System
		renderingSystem = gCoordinator.RegisterSystem<RenderingSystem>();
		{
			Signature sign;
			sign.set(gCoordinator.GetComponentType<Sprite>());
			sign.set(gCoordinator.GetComponentType<Transform>());
			gCoordinator.SetSystemSignature<RenderingSystem>(sign);
		}
		renderingSystem->Init(pGraphics, pResourcesManager, &gCoordinator);

		cameraSystem = gCoordinator.RegisterSystem<CameraSystem>();
		{
			Signature sign;
			sign.set(gCoordinator.GetComponentType<Camera>());
			sign.set(gCoordinator.GetComponentType<Transform>());
			gCoordinator.SetSystemSignature<CameraSystem>(sign);
		}
		cameraSystem->Init(&gCoordinator);

		// deserialize stuff
		m_LoadProgress = 0.5f;
		GameSerializer serializer;
		serializer.Register(&m_Coordinator);
		serializer.load(m_FilePath);
		m_LoadProgress = 1.0f;
	}

	void Scene::OnUnload()
	{
		if (m_State != SceneState::Loaded)
			return;

		m_State = SceneState::Unloading;

		// Wait for async load if still loading
		if (m_LoadFuture.valid())
			m_LoadFuture.wait();

		// Destroy all entities
		m_Coordinator.DestroyAllEntities();
		pResourcesManager->UnloadAllTextures();
		pResourcesManager->UnloadAllSound();

		m_State = SceneState::Unloaded;
		m_LoadProgress = 0.0f;
	}

	void Scene::OnUpdate(float dt)
	{
		if (m_State != SceneState::Loaded)
			return;
		// TODO: need handles for all systems
		// Update ECS systems
		playerController->Update(dt);
		physicsSystem->Update(smoothedDt);
		collisionSystem->Update(dt);
		cameraSystem->Update(dt);
		pGraphics->ClearBackground(0.2f, 0.3f, 0.3f);
		renderingSystem->Update(dt);
	}

	void Scene::OnRender()
	{
		//if (m_State != SceneState::Loaded)
		//	return;

		// Render this scene
	}

	Uma_ECS::Entity Scene::CreateEntity()
	{
		return m_Coordinator.CreateEntity();
	}

	void Scene::DestroyEntity(Uma_ECS::Entity entity)
	{
		m_Coordinator.DestroyEntity(entity);
	}

	void Scene::Serialize(const std::string& filepath)
	{
		GameSerializer serializer;
		serializer.Register(&m_Coordinator);
		serializer.save(filepath.empty() ? m_FilePath : filepath);
	}

	void Scene::Deserialize(const std::string& filepath)
	{
		GameSerializer serializer;
		serializer.Register(&m_Coordinator);
		serializer.load(filepath.empty() ? m_FilePath : filepath);
	}

    void Scene::SpawnDefaultEntities()
    {
        gCoordinator.DestroyAllEntities();

        using namespace Uma_ECS;

        Entity kappa;
        {
            kappa = gCoordinator.CreateEntity();

            gCoordinator.AddComponent(
                kappa,
                RigidBody{
                  .velocity = Vec2(0.0f, 0.0f),
                  .acceleration = Vec2(0.0f, 0.0f),
                  .accel_strength = 200,
                  .fric_coeff = 100
                });

            gCoordinator.AddComponent(
                kappa,
                Transform{
                  .position = Vec2(30, 35),
                  .rotation = Vec2(0, 0),
                  .scale = Vec2(3.f, 3.f)
                });

            std::string texName = "kappa_statue";
            gCoordinator.AddComponent(
                kappa,
                Sprite{
                  .textureName = texName,
                  .renderLayer = RL_ENV,
                  .flipX = false,
                  .flipY = false,
                  .UseNativeSize = true,
                  .texture = pResourcesManager->GetTexture(texName),
                });
        }

        Entity wall;
        {
            wall = gCoordinator.CreateEntity();

            gCoordinator.AddComponent(
                wall,
                RigidBody{
                  .velocity = Vec2(0.0f, 0.0f),
                  .acceleration = Vec2(0.0f, 0.0f),
                  .accel_strength = 200,
                  .fric_coeff = 100
                });

            gCoordinator.AddComponent(
                wall,
                Transform{
                  .position = Vec2(20, 0),
                  .rotation = Vec2(0, 0),
                  .scale = Vec2(1.f, 1.f)
                });

            std::string texName = "wall_top";
            gCoordinator.AddComponent(
                wall,
                Sprite{
                  .textureName = texName,
                  .renderLayer = RL_WALL,
                  .flipX = false,
                  .flipY = false,
                  .UseNativeSize = true,
                  .texture = pResourcesManager->GetTexture(texName),
                });

            // Create collider with two shapes
            Collider wallCollider;

            // Primary shape: Body hitbox (for taking damage)
            wallCollider.shapes[0] = ColliderShape{
                .purpose = ColliderPurpose::Environment,
                .layer = CL_WALL,
                .colliderMask = CL_PLAYER | CL_ENEMY,  // Blocks entities,
                .isActive = true,
                .autoFitToSprite = true  // Will be 128x128 (64*2 scale)
            };

            wallCollider.bounds.resize(wallCollider.shapes.size());
            gCoordinator.AddComponent(wall, wallCollider);

            for (size_t i = 0; i < 5; i++)
            {
                Entity tmp = gCoordinator.DuplicateEntity(wall);

                Transform& tf = gCoordinator.GetComponent<Transform>(tmp);

                tf.position = Vec2(20 + (i * 5), 0);

                Sprite& sr = gCoordinator.GetComponent<Sprite>(tmp);

                // set texture randomly
                sr.textureName = "wall_btm";
                sr.texture = pResourcesManager->GetTexture(sr.textureName);
            }

            for (size_t i = 0; i < 6; i++)
            {
                Entity tmp = gCoordinator.DuplicateEntity(wall);

                Transform& tf = gCoordinator.GetComponent<Transform>(tmp);

                tf.position = Vec2(15 + (6 * 5), 5 + (i * 5));

                Sprite& sr = gCoordinator.GetComponent<Sprite>(tmp);

                // set texture randomly
                sr.textureName = "wall_right";
                sr.texture = pResourcesManager->GetTexture(sr.textureName);
            }

            for (size_t i = 0; i < 5; i++)
            {
                Entity tmp = gCoordinator.DuplicateEntity(wall);

                Transform& tf = gCoordinator.GetComponent<Transform>(tmp);

                tf.position = Vec2(20 + (i * 5), 15 + (4 * 5));

                Sprite& sr = gCoordinator.GetComponent<Sprite>(tmp);

                // set texture randomly
                sr.textureName = "wall_top";
                sr.texture = pResourcesManager->GetTexture(sr.textureName);
            }
        }

        Entity floor;
        {
            floor = gCoordinator.CreateEntity();

            gCoordinator.AddComponent(
                floor,
                RigidBody{
                  .velocity = Vec2(0.0f, 0.0f),
                  .acceleration = Vec2(0.0f, 0.0f),
                  .accel_strength = 200,
                  .fric_coeff = 100
                });

            gCoordinator.AddComponent(
                floor,
                Transform{
                  .position = Vec2(20, 7.5),
                  .rotation = Vec2(0, 0),
                  .scale = Vec2(2.f, 2.f)
                });

            std::string texName = "floor_tatami";
            gCoordinator.AddComponent(
                floor,
                Sprite{
                  .textureName = texName,
                  .renderLayer = RL_WALL,
                  .flipX = false,
                  .flipY = false,
                  .UseNativeSize = true,
                  .texture = pResourcesManager->GetTexture(texName),
                });

            for (size_t i = 0; i < 5; i++)
            {
                for (size_t j = 0; j < 3; j++)
                {
                    Entity tmp = gCoordinator.DuplicateEntity(floor);

                    Transform& tf = gCoordinator.GetComponent<Transform>(tmp);

                    tf.position = Vec2(20 + (i * 5), 7.5 + (j * 10));
                }
            }
        }

        // create entities
        {
            Entity enemy;
            {
                enemy = gCoordinator.CreateEntity();

                gCoordinator.AddComponent(
                    enemy,
                    Enemy{
                        .mSpeed = 1.f
                    });

                gCoordinator.AddComponent(
                    enemy,
                    RigidBody{
                      .velocity = Vec2(0.0f, 0.0f),
                      .acceleration = Vec2(0.0f, 0.0f),
                      .accel_strength = 200,
                      .fric_coeff = 100
                    });

                gCoordinator.AddComponent(
                    enemy,
                    Transform{
                      .position = Vec2(-10, 0),
                      .rotation = Vec2(0, 0),
                      .scale = Vec2(2.f, 2.f)
                    });

                std::string texName = "pink_enemy";
                gCoordinator.AddComponent(
                    enemy,
                    Sprite{
                      .textureName = texName,
                      .renderLayer = RL_ENEMY,
                      .flipX = false,
                      .flipY = false,
                      .UseNativeSize = true,
                      .texture = pResourcesManager->GetTexture(texName),
                    });

                // Create collider with two shapes
                Collider enemyCollider;

                enemyCollider.shapes[0] = ColliderShape{
                    .size = Vec2(3.f, 3.f),
                    .offset = Vec2(0.f, 1.f),
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_ENEMY,
                    .colliderMask = CL_PLAYER | CL_PROJECTILE,
                    .isActive = true,
                    .autoFitToSprite = false
                };

                enemyCollider.shapes.push_back(ColliderShape{
                    .size = Vec2(2.f, 0.5f),
                    .offset = Vec2(0.f, -2.f),
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_ENEMY,
                    .colliderMask = CL_WALL,
                    .isActive = true,
                    .autoFitToSprite = false
                    });

                enemyCollider.bounds.resize(enemyCollider.shapes.size());
                gCoordinator.AddComponent(enemy, enemyCollider);
            }
        }

        // create player
        player = gCoordinator.CreateEntity();
        {
            gCoordinator.AddComponent(
                player,
                Transform
                {
                    .position = Vec2(0.f, 0.f),
                    .rotation = Vec2(0.f, 0.f),
                    .scale = Vec2(1,1),
                });

            gCoordinator.AddComponent(
                player,
                RigidBody{
                  .velocity = Vec2(0.0f, 0.0f),
                  .acceleration = Vec2(0.0f, 0.0f),
                  .accel_strength = 500,
                  .fric_coeff = 5
                });

            gCoordinator.AddComponent(
                player,
                Player{
                    .mSpeed = 1.f
                });

            std::string texName = "player";
            gCoordinator.AddComponent(
                player,
                Sprite{
                  .textureName = texName,
                  .renderLayer = RL_PLAYER,
                  .flipX = false,
                  .flipY = false,
                  .UseNativeSize = true,
                  .texture = pResourcesManager->GetTexture(texName),
                });

            // Create collider with two shapes
            Collider playerCollider;

            playerCollider.shapes[0] = ColliderShape{
                    .purpose = ColliderPurpose::Physics,
                    .layer = CL_PLAYER,
                    .colliderMask = CL_ENEMY | CL_PROJECTILE,
                    .isActive = true,
                    .autoFitToSprite = true
            };

            playerCollider.shapes.push_back(ColliderShape{
                .size = Vec2(7.0f, 0.5f),
                .offset = Vec2(0, -2.75f),
                .purpose = ColliderPurpose::Physics,
                .layer = CL_PLAYER,
                .colliderMask = CL_WALL,
                .isActive = true,
                .autoFitToSprite = false
                });

            playerCollider.bounds.resize(playerCollider.shapes.size());
            gCoordinator.AddComponent(player, playerCollider);
        }

        // create camera
        cam = gCoordinator.CreateEntity();
        {
            gCoordinator.AddComponent(
                cam,
                Transform
                {
                    .position = Vec2(400.0f, 300.0f),
                    .rotation = Vec2(0,0),
                    .scale = Vec2(1,1),
                });

            gCoordinator.AddComponent(
                player,
                Camera
                {
                    .mZoom = 1.f,
                    .followPlayer = true
                });
        }
    }
}