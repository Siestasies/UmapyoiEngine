-- BossPhase1: 3 elite enemies fight the player
-- Boss entity is invisible/inactive, elites are parented under it
-- Base script tracks elite deaths and triggers combine when all 3 die

ExposedVars = {
    -- No exposed vars needed; elites handle themselves
}

local animator = nil

function state_enter(entity)
    Log("Boss Phase 1: Elite guardians active")

    -- Boss stays hidden, elites do the fighting
    if HasSprite() then
        local spriteComp = GetSprite()
        spriteComp.visible = false
    end

    -- Disable boss rigid body so it doesn't interfere
    if HasRigidBody() then
        GetRigidBody().velocity = Vec2(0.0, 0.0)
    end

    if HasPathFinding() then
        GetPathFinding().enabled = false
    end
end

function state_update(entity, dt)
    -- Base script handles tracking elite deaths and transitioning to BossCombine
    -- Nothing to do here; elites run their own AI states
end

function state_exit(entity)
    Log("Boss Phase 1: All elites defeated")
end
