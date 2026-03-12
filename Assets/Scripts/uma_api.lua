---@meta
-- Uma Engine Lua API Annotations
-- Auto-generated for VS Code Lua Language Server (sumneko/LLS)
-- DO NOT require() this file — it is picked up automatically by LLS.

---@alias Entity integer

-- =============================================================================
--  Math Types
-- =============================================================================

---@class Vec2
---@field x number
---@field y number
---@operator add(Vec2): Vec2
---@operator sub(Vec2): Vec2
---@operator mul(number): Vec2
---@operator div(number): Vec2
local Vec2 = {}

---@overload fun(): Vec2
---@overload fun(x: number, y: number): Vec2
---@return Vec2
function Vec2(...) end

---@class Vec3
---@field x number
---@field y number
---@field z number
---@operator add(Vec3): Vec3
---@operator sub(Vec3): Vec3
---@operator mul(number): Vec3
local Vec3 = {}

---@overload fun(): Vec3
---@overload fun(x: number, y: number, z: number): Vec3
---@return Vec3
function Vec3(...) end

-- =============================================================================
--  Components
-- =============================================================================

---@class Transform
---@field position Vec2
---@field rotation Vec3
---@field scale Vec2
---@field worldPosition Vec2

---@class RigidBody
---@field velocity Vec2
---@field acceleration Vec2
---@field accel_strength number
---@field fric_coeff number

---@class Sprite
---@field texturePath string
---@field renderLayer integer
---@field renderOrder integer
---@field flipX boolean
---@field flipY boolean
---@field autoFlip boolean
---@field UseNativeSize boolean
---@field tintColor Vec3
---@field alpha number
---@field spriteSheetGrid Vec2
---@field spriteCell Vec2
---@field spriteOffset Vec2
local Sprite = {}
---@return Vec2 uvOffset, Vec2 uvSize
function Sprite:GetUVs() end

---@class SpriteMaterial
---@field effectName string
local SpriteMaterial = {}
---@param name string
---@param value number
function SpriteMaterial:SetFloat(name, value) end
---@param name string
---@param value Vec2
function SpriteMaterial:SetVec2(name, value) end
---@param name string
---@param value Vec3
function SpriteMaterial:SetVec3(name, value) end
---@param name string
---@param r number
---@param g number
---@param b number
---@param a number
function SpriteMaterial:SetVec4(name, r, g, b, a) end
---@param name string
---@param value integer
function SpriteMaterial:SetInt(name, value) end

---@class Camera
---@field zoom number
---@field followPlayer boolean

---@class PathFinding
---@field goal Vec2
---@field reachedGoal boolean
---@field enabled boolean

---@class Cutscene
---@field playOnce boolean
---@field hasPlayed boolean

-- =============================================================================
--  Combat / Stats
-- =============================================================================

---@enum ElementType
ElementType = {
    None = 0,
    Fire = 1,
    Water = 2,
    Wind = 3,
    Steam = 4,
    Pyronado = 5,
    Whirlpool = 6,
}

---@class AttackStats
---@field attackName string
---@field animationClipName string
---@field mDamageMultiplier number
---@field mAttackSpeedMultiplier number
---@field triggerColliderIndex integer
---@field manaCost number
---@field attackRange number
---@field attackArc number
---@field applyBurn boolean
---@field applyStun boolean
---@field effectDuration number
---@field attackCd number
---@field attackCdCurr number
---@field attackIsInCoolDown boolean
---@field elementType ElementType

---@class CheckpointData
---@field checkpointID integer
---@field checkpointX number
---@field checkpointY number
---@field hasCheckpoint boolean

---@class Player
---@field mHealth number
---@field mMaxHealth number
---@field mHealthRegenRate number
---@field mHealthRegenDelay number
---@field mHealthRegenDelayTimer number
---@field mCanRegenHealth boolean
---@field mSpeed number
---@field mDashSpeed number
---@field mDashDuration number
---@field mDashCD number
---@field mAttackDamage number
---@field mAttackSpeed number
---@field mAttackRange number
---@field mDefense number
---@field mMana number
---@field mMaxMana number
---@field mManaRegenRate number
---@field mNeutralAttackManaGain number
---@field isStunned boolean
---@field stunedTimer number
---@field isInvulnerable boolean
---@field mInvulnerabilityDuration number
---@field mHitStunDuration number
---@field lastElementUsed ElementType
---@field elementComboTimer number
---@field elementComboWindow number
---@field hasShield boolean
---@field isShieldBroken boolean
---@field currAttackIndex integer
---@field attackStats AttackStats[]
---@field checkpointData CheckpointData

---@class Enemy
---@field mHealth number
---@field mMaxHealth number
---@field mHealthRegenRate number
---@field mSpeed number
---@field mAttackDamage number
---@field mAttackSpeed number
---@field mAttackRange number
---@field mDefense number

---@enum ProjectileType
ProjectileType = {
    AOE = 0,
    SINGLE = 1,
}

---@class ProjectileStats
---@field type ProjectileType
---@field damage number
---@field speed number
---@field fadeOVerTime number
---@field lifeTime number

---@class Projectile
---@field mStats ProjectileStats

-- =============================================================================
--  Animation
-- =============================================================================

---@class SpriteAnimator
local SpriteAnimator = {}
---@param name string
---@param restart boolean
function SpriteAnimator:Play(name, restart) end
---@return boolean
function SpriteAnimator:IsPlaying() end
---@return boolean
function SpriteAnimator:HasFinished() end
---@return string
function SpriteAnimator:GetCurrentClip() end
---@return integer
function SpriteAnimator:GetCurrentFrame() end
function SpriteAnimator:Reset() end
---@overload fun(self: SpriteAnimator, name: string, framesX: integer, framesY: integer, startFrame: integer, frameCount: integer)
---@overload fun(self: SpriteAnimator, name: string, framesX: integer, framesY: integer, startFrame: integer, frameCount: integer, fps: number)
---@param name string
---@param framesX integer
---@param framesY integer
---@param startFrame integer
---@param frameCount integer
---@param fps number
---@param loop boolean
function SpriteAnimator:AddClip(name, framesX, framesY, startFrame, frameCount, fps, loop) end
---@param name string
---@return boolean
function SpriteAnimator:RemoveClip(name) end

---@class Animator
---@field autoPlay boolean
---@field initialClip string
---@field isInitialized boolean
---@field uvOffset Vec2
---@field uvSize Vec2
---@field animator SpriteAnimator

-- =============================================================================
--  Collision
-- =============================================================================

---@class BoundingBox
---@field min Vec2
---@field max Vec2

---@enum ColliderPurpose
ColliderPurpose = {
    Physics = 0,
    Environment = 1,
    Trigger = 2,
}

---@class ColliderShape
---@field size Vec2
---@field offset Vec2
---@field purpose integer
---@field layer integer
---@field colliderMask integer
---@field isActive boolean
---@field autoFitToSprite boolean

---@class Collider
---@field shapes ColliderShape[]
---@field bounds BoundingBox[]
---@field defaultLayer integer
---@field defaultMask integer
---@field showBBox boolean
local Collider = {}
---@return ColliderShape
function Collider:GetPrimaryShape() end
---@return BoundingBox
function Collider:GetPrimaryBounds() end
---@return integer
function Collider:GetEffectiveLayer() end
---@return integer
function Collider:GetEffectiveMask() end

---Collision layer constants
CollisionLayer = {
    NONE = 0,
    DEFAULT = 1,
    PLAYER = 2,
    ENEMY = 4,
    WALL = 8,
    PROJECTILE = 16,
    PICKUP = 32,
    ALL = 0xFFFFFFFF,
}

-- =============================================================================
--  UI
-- =============================================================================

---@class Color
---@field r number
---@field g number
---@field b number
---@field a number

---@class Text
---@field text string
---@field visible boolean

---@class Image
---@field textureName string
---@field sortingOrder integer
---@field color Color

---@enum SliderDirection
SliderDirection = {
    LeftToRight = 0,
    RightToLeft = 1,
    BottomToTop = 2,
    TopToBottom = 3,
}

---@class Slider
---@field minValue number
---@field maxValue number
---@field value number
---@field wholeNumbers boolean
---@field direction SliderDirection
---@field interactable boolean
---@field background string
---@field fill string
---@field handle string
---@field normalColour Color
---@field highlightColour Color
---@field disabledColour Color
---@field scriptName string
---@field isDragging boolean
---@field isHovered boolean

---@enum CheckboxState
CheckboxState = {
    Normal = 0,
    Hovered = 1,
    Pressed = 2,
    Disabled = 3,
}

---@class Checkbox
---@field isChecked boolean
---@field interactable boolean
---@field currentState CheckboxState
---@field background string
---@field checkmark string
---@field normalColour Color
---@field hoverColour Color
---@field pressedColour Color
---@field disabledColour Color
---@field checkedColour Color
---@field checkmarkNormalColour Color
---@field checkmarkDisabledColour Color
---@field scriptName string
---@field wasHoveredLastFrame boolean
---@field wasPressedWhileHovered boolean

---@class DialogueLine
---@field speaker string
---@field text string
---@field portrait string

---@class Dialogue
local Dialogue = {}
---@param seqId string
---@return DialogueLine[]|nil
function Dialogue:GetSequence(seqId) end
---@param seqId string
---@return integer
function Dialogue:GetLineCount(seqId) end
---@param seqId string
---@return boolean
function Dialogue:HasSequence(seqId) end

-- =============================================================================
--  Effects / Tweening
-- =============================================================================

---@enum EffectProperty
EffectProperty = {
    Position = 0,
    Scale = 1,
    ColorTint = 2,
    Alpha = 3,
}

---@enum EasingType
EasingType = {
    Linear = 0,
    EaseInQuad = 1,
    EaseOutQuad = 2,
    EaseInOutQuad = 3,
    EaseInCubic = 4,
    EaseOutCubic = 5,
    EaseInOutCubic = 6,
    EaseInQuart = 7,
    EaseOutQuart = 8,
    EaseInOutQuart = 9,
    EaseInElastic = 10,
    EaseOutElastic = 11,
    EaseInOutElastic = 12,
    EaseInBounce = 13,
    EaseOutBounce = 14,
    EaseInOutBounce = 15,
}

---@class EffectClip
---@field name string
---@field property EffectProperty
---@field easing EasingType
---@field duration number
---@field delay number
---@field loop boolean
---@field applyToChildren boolean
---@field startVec2 Vec2
---@field endVec2 Vec2
---@field startColor Color
---@field endColor Color
---@field startFloat number
---@field endFloat number
---@field currentTime number
---@field isPlaying boolean
---@field hasStarted boolean
local EffectClip = {}
function EffectClip:Play() end
function EffectClip:Pause() end
function EffectClip:Stop() end
function EffectClip:Reset() end
---@return number
function EffectClip:GetProgress() end
---@return boolean
function EffectClip:IsComplete() end

---@class Effects
---@field clips EffectClip[]
---@field playOnEnable boolean
local Effects = {}
---@overload fun(self: Effects, name: string)
---@param index integer
function Effects:Play(index) end
function Effects:PlayAll() end
function Effects:StopAll() end
function Effects:ResetAll() end
---@param index integer
function Effects:PauseClip(index) end
---@param index integer
function Effects:StopClip(index) end
---@param index integer
function Effects:ResetClip(index) end
---@param name string
function Effects:PauseClipByName(name) end
---@param name string
function Effects:StopClipByName(name) end
---@param name string
function Effects:ResetClipByName(name) end
---@overload fun(self: Effects, name: string): boolean
---@param index integer
---@return boolean
function Effects:IsClipPlaying(index) end
---@overload fun(self: Effects, name: string): boolean
---@param index integer
---@return boolean
function Effects:IsClipComplete(index) end
---@return string
function Effects:GetCurrentClip() end
---@return integer
function Effects:GetClipCount() end
---@param name string
---@return integer
function Effects:FindClipIndexByName(name) end
---@param clip EffectClip
function Effects:AddClip(clip) end
---@param name string
---@return boolean
function Effects:RemoveClip(name) end

-- =============================================================================
--  Particles
-- =============================================================================

---@enum EmitterMode
EmitterMode = {
    Burst = 0,
    Continuous = 1,
    ScreenFill = 2,
}

---@class Particle
---@field position Vec2
---@field velocity Vec2
---@field color Vec3
---@field scale Vec2
---@field rotation number
---@field rotationSpeed number
---@field lifetime number
---@field maxLifetime number
---@field age number
---@field opacity number
---@field baseOpacity number
---@field active boolean

---@class ParticleAppearance
---@field scaleRange Vec2
---@field startColor Vec3
---@field endColor Vec3
---@field colorLerp boolean
---@field randomOpacity boolean
---@field opacityRange Vec2
---@field rotateParticles boolean
---@field rotationSpeedRange Vec2

---@class FadeSettings
---@field fadeIn boolean
---@field fadeInDuration number
---@field fadeOut boolean
---@field fadeOutDuration number
---@field fadeAtEdges boolean
---@field edgeFadeDistance number

---@class ParticlePhysics
---@field speedRange Vec2
---@field lifetimeRange Vec2
---@field gravity Vec2
---@field drag number

---@class SpawnSettings
---@field spawnOffset Vec2
---@field spawnRadius number
---@field useEmissionCone boolean
---@field emissionAngle number
---@field emissionSpread number

---@class EmissionSettings
---@field emissionRate number
---@field loop boolean
---@field loopDelay number

---@class ScreenFillSettings
---@field velocityXRange Vec2
---@field velocityYRange Vec2
---@field spawnAtTop boolean
---@field spawnMargin number

---@class EmitterInstance
---@field name string
---@field mode EmitterMode
---@field maxParticles integer
---@field texturePath string
---@field isActive boolean
---@field renderLayer integer
---@field renderOrder integer
---@field appearance ParticleAppearance
---@field fade FadeSettings
---@field physics ParticlePhysics
---@field spawn SpawnSettings
---@field emission EmissionSettings
---@field screenFill ScreenFillSettings
---@field particles Particle[]
---@field initialized boolean
---@field emissionTimer number
---@field burstTimer number
local EmitterInstance = {}
function EmitterInstance:Play() end
function EmitterInstance:Stop() end
function EmitterInstance:StopAndClear() end
function EmitterInstance:Pause() end
function EmitterInstance:Resume() end
---@return boolean
function EmitterInstance:IsPlaying() end
---@return boolean
function EmitterInstance:HasActiveParticles() end
---@return integer
function EmitterInstance:GetActiveParticleCount() end

---@class ParticleEmitter
---@field emitters EmitterInstance[]
local ParticleEmitter = {}
---@overload fun(self: ParticleEmitter): integer
---@param name string
---@return integer
function ParticleEmitter:AddEmitter(name) end
---@param index integer
function ParticleEmitter:RemoveEmitter(index) end
---@overload fun(self: ParticleEmitter, name: string): EmitterInstance|nil
---@param index integer
---@return EmitterInstance|nil
function ParticleEmitter:GetEmitter(index) end
---@return integer
function ParticleEmitter:GetEmitterCount() end
function ParticleEmitter:PlayAll() end
function ParticleEmitter:StopAll() end

-- =============================================================================
--  Audio Component
-- =============================================================================

---@class AudioComponent
---@field defaultVolume number
local AudioComponent = {}
---@return boolean
function AudioComponent:hasLoadedSound() end
---@param entity Entity
---@param name string
function AudioComponent:play(entity, name) end
---@param entity Entity
---@param name string
function AudioComponent:playOneShot(entity, name) end
---@overload fun(self: AudioComponent, entity: Entity)
---@overload fun(self: AudioComponent, entity: Entity, name: string)
function AudioComponent:stop(entity, ...) end
---@param entity Entity
---@param x number
---@param y number
---@param name string
function AudioComponent:playAtPos(entity, x, y, name) end
---@param entity Entity
---@param name string
---@param fadeTime number
function AudioComponent:playFaded(entity, name, fadeTime) end
---@overload fun(self: AudioComponent, entity: Entity, name: string, fadeTime: number)
---@overload fun(self: AudioComponent, entity: Entity, fadeTime: number)
---@overload fun(self: AudioComponent, entity: Entity, name: string)
---@overload fun(self: AudioComponent, entity: Entity)
function AudioComponent:fadeOut(entity, ...) end
---@param entity Entity
---@param name string
---@param dulled boolean
function AudioComponent:toggleLowpass(entity, name, dulled) end

-- =============================================================================
--  Feedback
-- =============================================================================

---@enum FeedbackType
FeedbackType = {
    Normal = 0,
    Affinity = 1,
    Critical = 2,
    Heal = 3,
    PlayerHit = 4,
    ManaSpend = 5,
    ManaGain = 6,
    Warning = 7,
}

-- =============================================================================
--  Entity Wrapper
-- =============================================================================

---@class EntityWrapper
---@field id Entity
---@field isValid boolean
---@field GetTransform fun(): Transform|nil
---@field HasTransform fun(): boolean
---@field GetRigidBody fun(): RigidBody|nil
---@field HasRigidBody fun(): boolean
---@field GetSprite fun(): Sprite|nil
---@field HasSprite fun(): boolean
---@field GetCollider fun(): Collider|nil
---@field HasCollider fun(): boolean
---@field GetPlayer fun(): Player|nil
---@field HasPlayer fun(): boolean
---@field GetEnemy fun(): Enemy|nil
---@field HasEnemy fun(): boolean
---@field GetCamera fun(): Camera|nil
---@field HasCamera fun(): boolean
---@field GetPathFinding fun(): PathFinding|nil
---@field HasPathFinding fun(): boolean
---@field GetProjectile fun(): Projectile|nil
---@field HasProjectile fun(): boolean
---@field GetAnimator fun(): Animator|nil
---@field HasAnimator fun(): boolean
---@field GetText fun(): Text|nil
---@field HasText fun(): boolean
---@field GetImage fun(): Image|nil
---@field HasImage fun(): boolean
---@field GetEffects fun(): Effects|nil
---@field HasEffects fun(): boolean
---@field GetParticleEmitter fun(): ParticleEmitter|nil
---@field HasParticleEmitter fun(): boolean
---@field GetAudioComponent fun(): AudioComponent|nil
---@field HasAudioComponent fun(): boolean
---@field GetSlider fun(): Slider|nil
---@field HasSlider fun(): boolean
---@field GetCheckbox fun(): Checkbox|nil
---@field HasCheckbox fun(): boolean
---@field GetSpriteMaterial fun(): SpriteMaterial|nil
---@field HasSpriteMaterial fun(): boolean
---@field GetCutscene fun(): Cutscene|nil
---@field HasCutscene fun(): boolean

-- =============================================================================
--  Leaderboard (PlayFab)
-- =============================================================================

---@class LeaderboardEntry
---@field entityId string
---@field displayName string
---@field rank integer
---@field score number

-- =============================================================================
--  Global Functions — Entity Management
-- =============================================================================

---@return Entity
function CreateEntity() end

---@param entity Entity
function DestroyEntity(entity) end

---@param child Entity
---@param parent Entity
function SetParent(child, parent) end

---@param child Entity
function RemoveParent(child) end

---@param entity Entity
---@return integer  @ parent entity ID, or -1 if no parent
function GetParent(entity) end

---@param entity Entity
---@return boolean
function HasParent(entity) end

---@param entity Entity
---@return Entity[]
function GetChildrenList(entity) end

---@param entity Entity
---@param index integer
---@return Entity
function GetChildren(entity, index) end

---@param entity Entity
---@param index integer
---@return boolean
function HasChildren(entity, index) end

---@param entity Entity
function DestroyWithChildren(entity) end

---@param entity Entity
---@param isActive boolean
function SetActiveEntity(entity, isActive) end

---@param entity Entity
---@return boolean
function GetActiveEntity(entity) end

-- =============================================================================
--  Global Functions — Entity Queries
-- =============================================================================

---@param componentName string
---@return Entity[]
function FindEntitiesWithComponent(componentName) end

---@param componentName string
---@return integer
function CountEntitiesWithComponent(componentName) end

---@param componentName string
---@return Entity
function FindEntityWithComponent(componentName) end

---@return integer
function GetEntityCount() end

---@param entity Entity
---@return boolean
function IsEntityValid(entity) end

---@param targetEntity Entity
---@return EntityWrapper
function GetEntity(targetEntity) end

-- =============================================================================
--  Global Functions — Direct Component Access
-- =============================================================================

---@param entity Entity
---@return Transform|nil
function GetTransformFrom(entity) end
---@param entity Entity
---@return boolean
function HasTransformOn(entity) end

---@param entity Entity
---@return RigidBody|nil
function GetRigidBodyFrom(entity) end
---@param entity Entity
---@return boolean
function HasRigidBodyOn(entity) end

---@param entity Entity
---@return Sprite|nil
function GetSpriteFrom(entity) end
---@param entity Entity
---@return boolean
function HasSpriteOn(entity) end

---@param entity Entity
---@return Collider|nil
function GetColliderFrom(entity) end
---@param entity Entity
---@return boolean
function HasColliderOn(entity) end

---@param entity Entity
---@return Player|nil
function GetPlayerFrom(entity) end
---@param entity Entity
---@return boolean
function HasPlayerOn(entity) end

---@param entity Entity
---@return Enemy|nil
function GetEnemyFrom(entity) end
---@param entity Entity
---@return boolean
function HasEnemyOn(entity) end

---@param entity Entity
---@return Camera|nil
function GetCameraFrom(entity) end
---@param entity Entity
---@return boolean
function HasCameraOn(entity) end

---@param entity Entity
---@return PathFinding|nil
function GetPathFindingFrom(entity) end
---@param entity Entity
---@return boolean
function HasPathFindingOn(entity) end

---@param entity Entity
---@return Projectile|nil
function GetProjectileFrom(entity) end
---@param entity Entity
---@return boolean
function HasProjectileOn(entity) end

---@param entity Entity
---@return Animator|nil
function GetAnimatorFrom(entity) end
---@param entity Entity
---@return boolean
function HasAnimatorOn(entity) end

---@param entity Entity
---@return Text|nil
function GetTextFrom(entity) end
---@param entity Entity
---@return boolean
function HasTextOn(entity) end

---@param entity Entity
---@return Image|nil
function GetImageFrom(entity) end
---@param entity Entity
---@return boolean
function HasImageOn(entity) end

---@param entity Entity
---@return Effects|nil
function GetEffectsFrom(entity) end
---@param entity Entity
---@return boolean
function HasEffectsOn(entity) end

---@param entity Entity
---@return ParticleEmitter|nil
function GetParticleEmitterFrom(entity) end
---@param entity Entity
---@return boolean
function HasParticleEmitterOn(entity) end

---@param entity Entity
---@return AudioComponent|nil
function GetAudioComponentFrom(entity) end
---@param entity Entity
---@return boolean
function HasAudioComponentOn(entity) end

---@param entity Entity
---@return Slider|nil
function GetSliderFrom(entity) end
---@param entity Entity
---@return boolean
function HasSliderOn(entity) end

---@param entity Entity
---@return Checkbox|nil
function GetCheckboxFrom(entity) end
---@param entity Entity
---@return boolean
function HasCheckboxOn(entity) end

---@param entity Entity
---@return SpriteMaterial|nil
function GetSpriteMaterialFrom(entity) end
---@param entity Entity
---@return boolean
function HasSpriteMaterialOn(entity) end

---@param entity Entity
---@return Cutscene|nil
function GetCutsceneFrom(entity) end
---@param entity Entity
---@return boolean
function HasCutsceneOn(entity) end

---@param entity Entity
---@return Dialogue|nil
function GetDialogueFrom(entity) end
---@param entity Entity
---@return boolean
function HasDialogueOn(entity) end

-- =============================================================================
--  Global Functions — Animation / Physics
-- =============================================================================

---@param entity Entity
---@param name string
function PlayAnimation(entity, name) end

---@param entity Entity
---@param pos Vec2
---@param dir Vec2
---@param force number
---@param rotation number
function AddForce(entity, pos, dir, force, rotation) end

---@param entity Entity
---@param x number
---@param y number
function SetPathFindingGoal(entity, x, y) end

-- =============================================================================
--  Global Functions — Prefab / Spawning
-- =============================================================================

---@param prefabName string  @ include file extension e.g. "Bullet.prefab"
---@param pos Vec2
---@return Entity
function SpawnPrefab(prefabName, pos) end

-- =============================================================================
--  Global Functions — Dialogue / Cutscene
-- =============================================================================

---@param entity Entity
---@param seqId string
---@return table|nil  @ array of {speaker, text, portrait}
function GetDialogueSequence(entity, seqId) end

---@param entity Entity
---@return table|nil  @ array of cutscene action tables
function GetCutsceneActions(entity) end

---@param entity Entity
---@param played boolean
function SetCutscenePlayed(entity, played) end

---@param active boolean
function SetCutsceneActive(active) end

---@return boolean
function IsCutsceneActive() end

---Cutscene action type constants
---@type integer
CUTSCENE_SET_CAMERA = 0
---@type integer
CUTSCENE_LERP_CAMERA = 1
---@type integer
CUTSCENE_PLAY_DIALOGUE = 2
---@type integer
CUTSCENE_WAIT = 3
---@type integer
CUTSCENE_RETURN_CAMERA = 4
---@type integer
CUTSCENE_SHAKE_CAMERA = 5

-- =============================================================================
--  Global Functions — Camera
-- =============================================================================

---@param cameraEntity Entity
---@param follow boolean
function SetCameraFollow(cameraEntity, follow) end

---@param cameraEntity Entity
---@param x number
---@param y number
function SetCameraPosition(cameraEntity, x, y) end

---@param entity Entity
---@return number x, number y
function GetTransformPosition(entity) end

---@param cameraEntity Entity
---@param intensity number
---@param duration number
function CameraShake(cameraEntity, intensity, duration) end

---@param cameraEntity Entity
---@param zoom number
function SetCameraZoom(cameraEntity, zoom) end

---@param cameraEntity Entity
---@return number
function GetCameraZoom(cameraEntity) end

---@return Entity  @ camera entity or -1
function FindCameraEntity() end

-- =============================================================================
--  Global Functions — Logging
-- =============================================================================

---@param msg string
function Log(msg) end

---@param msg string
function LogWarning(msg) end

---@param msg string
function LogError(msg) end

-- =============================================================================
--  Global Functions — Time / Application
-- =============================================================================

---@return number
function GetDeltaTime() end

---@return number
function GetFps() end

---@param isPause boolean
function PauseGame(isPause) end

---@return boolean
function IsGamePause() end

function CloseApplication() end

-- =============================================================================
--  Global Functions — Scene Management
-- =============================================================================

---@param sceneName string
function LoadScene(sceneName) end

function ReloadScene() end

function RestartScene() end

-- =============================================================================
--  Global Functions — FSM
-- =============================================================================

---@param entity Entity
---@param nextState string
function ChangeState(entity, nextState) end

-- =============================================================================
--  Global Functions — Audio (Global)
-- =============================================================================

---@param audioName string
---@param vol number
---@param loop integer
function PlaySound(audioName, vol, loop) end

---@param audioName string
function StopSound(audioName) end

---@param audioName string
---@param vol number
---@param loop integer
function PlayMusic(audioName, vol, loop) end

---@param audioName string
function StopMusic(audioName) end

---@param entity Entity
---@param audioName string
---@param loop boolean
---@param vol number
function PlayEntitySound(entity, audioName, loop, vol) end

---@param entity Entity
function StopEntitySound(entity) end

---@param entity Entity
---@param soundName string
function StopEntitySoundByName(entity, soundName) end

---@param entity Entity
---@param audioName string
---@param vol number
function PlayOneShotAtEntity(entity, audioName, vol) end

---@param entity Entity
---@param x number
---@param y number
---@param audioName string
---@param vol number
function PlayOneShotAtPosition(entity, x, y, audioName, vol) end

---@param volume number
function setMasterVolume(volume) end

---@param volume number
function setSFXVolume(volume) end

---@param volume number
function setBGMVolume(volume) end

---@param volume number
---@param type integer
function setGroupVolume(volume, type) end

---@param groupName string  @ "SFX", "BGM", or "MASTER"
---@param enable boolean
function toggleGroupLowpass(groupName, enable) end

-- =============================================================================
--  Global Functions — Feedback
-- =============================================================================

---@param worldX number
---@param worldY number
---@param value string
---@param typeStr? string  @ "Normal","Affinity","Critical","Heal","PlayerHit","ManaSpend","ManaGain","Warning"
function SpawnFeedback(worldX, worldY, value, typeStr) end

-- =============================================================================
--  Global Functions — Attack Stats
-- =============================================================================

---@return AttackStats
function CreateAttackStats() end

---@param player Player
---@param attack AttackStats
function AddAttackStats(player, attack) end

---@param player Player
function ClearAttackStats(player) end

---@param player Player
---@return integer
function GetAttackStatsCount(player) end

-- =============================================================================
--  Global Functions — Input
-- =============================================================================

---@return Vec2
function GetMousePosition() end

---@return Vec2
function GetMouseWorldPosition() end

---@param keyCode integer
---@return boolean
function KeyDown(keyCode) end

---@param keyCode integer
---@return boolean
function KeyPressed(keyCode) end

---@param keyCode integer
---@return boolean
function KeyReleased(keyCode) end

---@param button integer
---@return boolean
function MouseButtonDown(button) end

---@param button integer
---@return boolean
function MouseButtonPressed(button) end

---@param button integer
---@return boolean
function MouseButtonReleased(button) end

-- Key Constants
---@type integer
KEY_W = 0
---@type integer
KEY_A = 0
---@type integer
KEY_S = 0
---@type integer
KEY_D = 0
---@type integer
KEY_E = 0
---@type integer
KEY_Q = 0
---@type integer
KEY_R = 0
---@type integer
KEY_F = 0
---@type integer
KEY_G = 0
---@type integer
KEY_H = 0
---@type integer
KEY_I = 0
---@type integer
KEY_J = 0
---@type integer
KEY_K = 0
---@type integer
KEY_L = 0
---@type integer
KEY_M = 0
---@type integer
KEY_N = 0
---@type integer
KEY_O = 0
---@type integer
KEY_P = 0
---@type integer
KEY_T = 0
---@type integer
KEY_U = 0
---@type integer
KEY_V = 0
---@type integer
KEY_X = 0
---@type integer
KEY_Y = 0
---@type integer
KEY_Z = 0
---@type integer
KEY_B = 0
---@type integer
KEY_C = 0
---@type integer
KEY_0 = 0
---@type integer
KEY_1 = 0
---@type integer
KEY_2 = 0
---@type integer
KEY_3 = 0
---@type integer
KEY_4 = 0
---@type integer
KEY_5 = 0
---@type integer
KEY_6 = 0
---@type integer
KEY_7 = 0
---@type integer
KEY_8 = 0
---@type integer
KEY_9 = 0
---@type integer
KEY_SPACE = 0
---@type integer
KEY_SHIFT = 0
---@type integer
KEY_CTRL = 0
---@type integer
KEY_ESCAPE = 0
---@type integer
KEY_ENTER = 0
---@type integer
KEY_TAB = 0
---@type integer
KEY_BACKSPACE = 0
---@type integer
KEY_DELETE = 0
---@type integer
KEY_F1 = 0
---@type integer
KEY_F2 = 0
---@type integer
KEY_F3 = 0
---@type integer
KEY_F4 = 0
---@type integer
KEY_F5 = 0
---@type integer
KEY_F6 = 0
---@type integer
KEY_F7 = 0
---@type integer
KEY_F8 = 0
---@type integer
KEY_F9 = 0
---@type integer
KEY_F10 = 0
---@type integer
KEY_F11 = 0
---@type integer
KEY_F12 = 0
---@type integer
MOUSE_LEFT = 0
---@type integer
MOUSE_RIGHT = 0
---@type integer
MOUSE_MIDDLE = 0

-- =============================================================================
--  Global Functions — PlayFab
-- =============================================================================

---@return boolean
function PlayFab_IsReady() end

---@return boolean
function PlayFab_IsLoggedIn() end

---@param customId string
---@param createAccount boolean
---@param onSuccess? fun()
---@param onFailure? fun(msg: string)
function PlayFab_LoginWithCustomID(customId, createAccount, onSuccess, onFailure) end

function PlayFab_Logout() end

---@param onSuccess? fun(data: table<string, string>)  @ {playFabId, username, created}
---@param onFailure? fun(msg: string)
function PlayFab_GetAccountInfo(onSuccess, onFailure) end

---@param name string
---@param onSuccess? fun()
---@param onFailure? fun(msg: string)
function PlayFab_SetDisplayName(name, onSuccess, onFailure) end

---@param key string
---@param onSuccess? fun(value: string)
---@param onFailure? fun(msg: string)
function PlayFab_ReadData(key, onSuccess, onFailure) end

---@param keys string[]
---@param onSuccess? fun(data: table<string, string>)
---@param onFailure? fun(msg: string)
function PlayFab_ReadMultipleData(keys, onSuccess, onFailure) end

---@param key string
---@param value string
---@param onSuccess? fun()
---@param onFailure? fun(msg: string)
function PlayFab_WriteData(key, value, onSuccess, onFailure) end

---@param key string
---@param onSuccess? fun(value: string)
---@param onFailure? fun(msg: string)
function PlayFab_GetTitleData(key, onSuccess, onFailure) end

---@param statName string
---@param value number
---@param onSuccess? fun()
---@param onFailure? fun(msg: string)
function PlayFab_SubmitScore(statName, value, onSuccess, onFailure) end

---@param statName string
---@param onSuccess? fun(statName: string, value: number)
---@param onFailure? fun(msg: string)
function PlayFab_GetMyStats(statName, onSuccess, onFailure) end

---@param name string
---@param pageSize integer
---@param startPos integer
---@param onSuccess? fun(entries: LeaderboardEntry[])
---@param onFailure? fun(msg: string)
function PlayFab_GetLeaderboard(name, pageSize, startPos, onSuccess, onFailure) end

---@param name string
---@param pageSize integer
---@param onSuccess? fun(entries: LeaderboardEntry[])
---@param onFailure? fun(msg: string)
function PlayFab_GetLeaderboardAroundMe(name, pageSize, onSuccess, onFailure) end

-- =============================================================================
--  Script Lifecycle (implement these in your scripts)
-- =============================================================================

---Called once when the script is first enabled
function Start() end

---Called every frame
---@param dt number  @ delta time in seconds
function Update(dt) end

---Called when the entity is destroyed
function OnDestroy() end

---Called when this entity collides with another (first frame)
---@param other Entity
function OnCollisionEnter(other) end

---Called every frame while colliding
---@param other Entity
function OnCollision(other) end

---Called when collision ends
---@param other Entity
function OnCollisionExit(other) end

---Called when a trigger overlap begins
---@param other Entity
---@param triggerOwner Entity
function OnTriggerEnter(other, triggerOwner) end

---Called every frame while trigger overlaps
---@param other Entity
---@param triggerOwner Entity
function OnTrigger(other, triggerOwner) end

---Called when trigger overlap ends
---@param other Entity
function OnTriggerExit(other) end

-- Built-in variable available in every script environment
---@type Entity
EntityID = 0
