-- ====================================================================
-- FILE: test_lua_2_0.lua
-- PROJECT: GAM200 - Uma Engine
-- COURSE: CSD2401
-- SECTION: A
-- SOFTWARE ENGINEERING PROJECT 3
--
-- AUTHOR: Leong Wai Men (100%)
-- EMAIL: waimen.leong@digipen.edu
-- DIGIPEN LOGIN: waimen.leong
--
-- BRIEF:
-- Comprehensive test suite for Uma Engine's Lua scripting system integration.
-- Tests all major API categories including component access, entity queries,
-- cross-entity manipulation, input handling, collision callbacks, and vector
-- operations. Validates Sol2 bindings for Transform, RigidBody, Sprite, 
-- Collider, Player, Enemy, and Camera components. Demonstrates exposed variable
-- system with runtime modification via ImGui. Includes collision layer constants,
-- input key constants, and ColliderShape vector access patterns. Serves as
-- reference implementation for gameplay scripting and API usage validation.
--
-- TEST CATEGORIES:
-- 1. Entity Information & Exposed Variables
-- 2. Component Access (Has/Get pattern for self)
-- 3. Entity Queries (Find by component type)
-- 4. Cross-Entity Access (Direct GetXXXFrom functions)
-- 5. Entity Wrapper Pattern (GetEntity with methods)
-- 6. Collision Layer & Purpose Constants
-- 7. Input Key Constants & Bindings
-- 8. Vec2 Math Operations (commented)
-- 9. Runtime Input Testing (Movement, Collider modification)
-- 10. Collision/Trigger Callbacks (commented examples)
--
-- IMPORTANT : SOME OF THE CODE GOT COMMENTED OUT NOT BECAUSE ITS NOT WORKING 
-- BUT TO NOT POLLUTE THE GAME INPUT, SO IF U WANT TO TEST CAN UNCOMMENT THE CODE
--
-- USAGE:
-- Attach to any entity with LuaScript component. Press 'I' for runtime info,
-- IJKL for movement, E for collider debug, M for shape modification. Check
-- console output for comprehensive API validation results.
--
-- ALL CONTENT (C) 2025 DigiPen INSTITUTE OF TECHNOLOGY SINGAPORE.
-- ALL RIGHTS RESERVED.
-- ====================================================================

ExposedVars = {
    name = "Test_lua",
}

function Start()
    Log("========================================")
    Log("SCRIPT START")
    Log("========================================")
end

function Update(dt)
    --Log("========================================")
    --Log("SCRIPT UPDATE")
    --Log("========================================")
end

function OnEnable()
     Log("========================================")
    Log("SCRIPT ENABLED")
    Log("========================================")
end

function OnDisable()
     Log("========================================")
    Log("SCRIPT DISABLED")
    Log("========================================")
end

function OnDestroy()
     Log("========================================")
    Log("SCRIPT DESTROYED")
    Log("========================================")
end