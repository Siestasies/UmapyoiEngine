/*!
\file   Player.h
\par    Project: GAM250
\par    Course: CSD2451
\par    Section A
\par    Software Engineering Project 4

\author Leong Wai Men (100%)
\par    E-mail: waimen.leong@digipen.edu
\par    DigiPen login: waimen.leong

\brief
Defines Player tag component with speed parameter for entity identification in game systems.

Marker component enabling systems to distinguish player entity from other entities for input handling and camera tracking.
Includes JSON serialization/deserialization for mSpeed property. Contains design notes for event-driven architecture
using state-based continuous input and event-based skill/damage systems with UI subscriptions.

All content (C) 2025 DigiPen Institute of Technology Singapore.
All rights reserved.
*/

#pragma once
#include <algorithm>

namespace Uma_ECS
{
    enum AnimatorState
    {
        PS_Idle,
        PS_Run,
        PS_Dash,
        PS_Atk_1,
        PS_Atk_2,
        PS_Fire_Slash,
        PS_Water_Slash,
        PS_Steam_Burst,
        PS_Throw,
        PS_Hurt,
        PS_Die
    };

    // Element types for elemental system
    enum class ElementType
    {
        None,
        Fire,
        Water,
        Wind,
        Steam,
        Pyronado,
        Whirlpool
    };

    struct CombatState
    {
        //// Attack cooldowns
        //bool attack_1_is_in_cd = false;
        //bool attack_2_is_in_cd = false;
        //float attack_1_cd = 0.f;
        //float attack_2_cd = 0.f;
        //float attack_1_cd_curr = 0.f;
        //float attack_2_cd_curr = 0.f;

        //// Ability cooldowns
        //bool dash_is_in_cd = false;
        //float dash_cd = 2.f;
        //float dash_cd_curr = 0.f;

        //bool fire_slash_is_in_cd = false;
        //float fire_slash_cd = 1.5f;
        //float fire_slash_cd_curr = 0.f;

        //bool water_slash_is_in_cd = false;
        //float water_slash_cd = 1.5f;
        //float water_slash_cd_curr = 0.f;

        // State flags
        bool isAlive = true;
        //bool isStunned = false;
        //bool isInvulnerable = false;

        //// Timers
        //float stunedDuration = 0.f;
        //float stunedTimer = 0.f;
        //float invulnerabilityDuration = 1.f;
        //float invulnerabilityTimer = 0.f;

        //// Elemental combo system
        //ElementType lastElementUsed = ElementType::None;
        //float elementComboTimer = 0.f;
        //float elementComboWindow = 2.f;  // Time window to trigger fusion
    };

    struct AttackStats
    {
        std::string AttackName;
        std::string animationClipName;

        float mDamageMultiplier = 1.f;
        float mAttackSpeedMultiplier = 1.f;
        int triggerColliderIndex = 0;

        // Element type for this attack
        ElementType elementType = ElementType::None;

        // Mana cost (0 for neutral attacks)
        int manaCost = 0;

        // Attack shape
        float attackRange = 50.f;
        float attackArc = 90.f;  // Degrees for cone attacks

        // Special effects
        bool applyBurn = false;      // Fire attacks
        bool applyStun = false;      // Water attacks vs wind enemies
        float effectDuration = 0.f;

        // Runtime data
        float attackCd = 0.f;
        float attackCdCurr = 0.f;
        bool attackIsInCoolDown = false;
    };

    //struct ThrowableInventory
    //{
    //    int explosiveKunaiCount = 2;
    //    int explosiveKunaiMax = 2;

    //    // Future throwables can be added here
    //    // int elementalKunaiCount = 0;
    //    // int elementalKunaiMax = 0;
    //};

    struct CheckpointData
    {
        int checkpointID = 0;
        float checkpointX = 0.f;
        float checkpointY = 0.f;
        bool hasCheckpoint = false;
    };

    struct Player
    {
        // ===== HEALTH SYSTEM =====
        int     mHealth = 100;
        int     mMaxHealth = 100;
        float   mHealthRegenRate = 1.f;
        float   mHealthRegenDelay = 3.f;        // Delay after damage before regen
        float   mHealthRegenDelayTimer = 0.f;   // Current delay timer
        bool    mCanRegenHealth = true;

        // ===== MOVEMENT =====
        float   mSpeed = 50.f;
        float   mDashSpeed = 10.f;              // Multiplier for dash
        float   mDashDuration = 0.2f;           // How long dash lasts
        float   mDashCD = 2.f;
        float   mDashCDMax = 2.f;

        // ===== BASIC COMBAT =====
        int     mAttackDamage = 10;
        float   mAttackSpeed = 1.f;
        float   mAttackRange = 20.f;
        int     mDefense = 5;
        float   mCritDamage = 1.0f;

        // ===== MANA SYSTEM =====
        float   mMana = 100.f;
        float   mMaxMana = 100.f;
        float   mManaRegenRate = 5.f;           // Passive regen per second
        int     mNeutralAttackManaGain = 5;     // 50/50 chance to gain on hit

        // Status
        float stunedTimer = 0.f;
        bool isStunned = false;
        bool isInvulnerable = false;

        // ===== INVULNERABILITY =====
        float   mInvulnerabilityDuration = 1.f; // i-frames duration
        float   mHitStunDuration = 0.3f;        // Hit-stun duration

        // Elemental combo system
        ElementType lastElementUsed = ElementType::None;
        float elementComboTimer = 0.f;
        float elementComboWindow = 2.f;  // Time window to trigger fusion

        // ===== RUNTIME DATA =====
        int currAttackIndex = 0;
        AnimatorState animatorState = PS_Idle;
        std::vector<AttackStats> attackStats;
        CombatState combatState;
        //ThrowableInventory throwableInventory;
        CheckpointData checkpointData;
        Vec2 lookDir;
        bool hasShield;
        bool isShieldBroken;

        /*!
        \brief Serialize player data to JSON, including health, movement, combat, mana, and attack stats.
        \param value Output JSON value to populate.
        \param allocator RapidJSON allocator for creating new values.
        */
        void Serialize(rapidjson::Value& value, rapidjson::Document::AllocatorType& allocator) const
        {
            value.SetObject();

            // Health
            value.AddMember("mHealth", mHealth, allocator);
            value.AddMember("mMaxHealth", mMaxHealth, allocator);
            value.AddMember("mHealthRegenRate", mHealthRegenRate, allocator);
            value.AddMember("mHealthRegenDelay", mHealthRegenDelay, allocator);

            // Movement
            value.AddMember("mSpeed", mSpeed, allocator);
            value.AddMember("mDashSpeed", mDashSpeed, allocator);
            value.AddMember("mDashDuration", mDashDuration, allocator);
            value.AddMember("mDashCD", mDashCD, allocator);
            value.AddMember("mDashCDMax", mDashCDMax, allocator);

            // Combat
            value.AddMember("mAttackDamage", mAttackDamage, allocator);
            value.AddMember("mAttackSpeed", mAttackSpeed, allocator);
            value.AddMember("mAttackRange", mAttackRange, allocator);
            value.AddMember("mDefense", mDefense, allocator);
            value.AddMember("mCritDamage", mCritDamage, allocator);

            // Mana
            value.AddMember("mMana", mMana, allocator);
            value.AddMember("mMaxMana", mMaxMana, allocator);
            value.AddMember("mManaRegenRate", mManaRegenRate, allocator);
            value.AddMember("mNeutralAttackManaGain", mNeutralAttackManaGain, allocator);
            /*value.AddMember("mFireSlashManaCost", mFireSlashManaCost, allocator);
            value.AddMember("mWaterSlashManaCost", mWaterSlashManaCost, allocator);
            value.AddMember("mSteamBurstManaCost", mSteamBurstManaCost, allocator);*/

            // Invulnerability
            value.AddMember("mInvulnerabilityDuration", mInvulnerabilityDuration, allocator);
            value.AddMember("mHitStunDuration", mHitStunDuration, allocator);

            // Throwable inventory
            //value.AddMember("explosiveKunaiCount", throwableInventory.explosiveKunaiCount, allocator);
            //value.AddMember("explosiveKunaiMax", throwableInventory.explosiveKunaiMax, allocator);

            // Checkpoint
            value.AddMember("checkpointX", checkpointData.checkpointX, allocator);
            value.AddMember("checkpointY", checkpointData.checkpointY, allocator);
            value.AddMember("hasCheckpoint", checkpointData.hasCheckpoint, allocator);

            // Combat state - elemental combo
            value.AddMember("elementComboWindow", elementComboWindow, allocator);

            // Serialize attackStats
            rapidjson::Value attacksArray(rapidjson::kArrayType);
            for (const auto& attack : attackStats)
            {
                rapidjson::Value attackObj(rapidjson::kObjectType);

                rapidjson::Value nameVal;
                nameVal.SetString(attack.AttackName.c_str(), static_cast<rapidjson::SizeType>(attack.AttackName.length()), allocator);
                attackObj.AddMember("AttackName", nameVal, allocator);

                attackObj.AddMember("mDamageMultiplier", attack.mDamageMultiplier, allocator);
                attackObj.AddMember("mAttackSpeedMultiplier", attack.mAttackSpeedMultiplier, allocator);

                rapidjson::Value animClipVal;
                animClipVal.SetString(attack.animationClipName.c_str(), static_cast<rapidjson::SizeType>(attack.animationClipName.length()), allocator);
                attackObj.AddMember("animationClipName", animClipVal, allocator);

                attackObj.AddMember("triggerColliderIndex", attack.triggerColliderIndex, allocator);

                // New fields
                attackObj.AddMember("elementType", static_cast<int>(attack.elementType), allocator);
                attackObj.AddMember("manaCost", attack.manaCost, allocator);
                attackObj.AddMember("attackRange", attack.attackRange, allocator);
                attackObj.AddMember("attackArc", attack.attackArc, allocator);
                attackObj.AddMember("applyBurn", attack.applyBurn, allocator);
                attackObj.AddMember("applyStun", attack.applyStun, allocator);
                attackObj.AddMember("effectDuration", attack.effectDuration, allocator);

                attacksArray.PushBack(attackObj, allocator);
            }
            value.AddMember("attackStats", attacksArray, allocator);
        }

        /*!
        \brief Deserialize player data from JSON, restoring all stats and initializing runtime data.
        \param value JSON value containing serialized player data.
        */
        void Deserialize(const rapidjson::Value& value)
        {
            // Health
            if (value.HasMember("mHealth"))
                mHealth = value["mHealth"].GetInt();
            if (value.HasMember("mMaxHealth"))
                mMaxHealth = value["mMaxHealth"].GetInt();
            if (value.HasMember("mHealthRegenRate"))
                mHealthRegenRate = value["mHealthRegenRate"].GetFloat();
            if (value.HasMember("mHealthRegenDelay"))
                mHealthRegenDelay = value["mHealthRegenDelay"].GetFloat();

            // Movement
            if (value.HasMember("mSpeed"))
                mSpeed = value["mSpeed"].GetFloat();
            if (value.HasMember("mDashSpeed"))
                mDashSpeed = value["mDashSpeed"].GetFloat();
            if (value.HasMember("mDashDuration"))
                mDashDuration = value["mDashDuration"].GetFloat();
            if (value.HasMember("mDashCD"))
                mDashCD = value["mDashCD"].GetFloat();
            if (value.HasMember("mDashCDMax"))
                mDashCDMax = value["mDashCDMax"].GetFloat();

            // Combat
            if (value.HasMember("mAttackDamage"))
                mAttackDamage = value["mAttackDamage"].GetInt();
            if (value.HasMember("mAttackSpeed"))
                mAttackSpeed = value["mAttackSpeed"].GetFloat();
            if (value.HasMember("mAttackRange"))
                mAttackRange = value["mAttackRange"].GetFloat();
            if (value.HasMember("mDefense"))
                mDefense = value["mDefense"].GetInt();
            if (value.HasMember("mCritDamage"))
                mCritDamage = value["mCritDamage"].GetFloat();

            // Mana
            if (value.HasMember("mMana"))
                mMana = value["mMana"].GetFloat();
            if (value.HasMember("mMaxMana"))
                mMaxMana = value["mMaxMana"].GetFloat();
            if (value.HasMember("mManaRegenRate"))
                mManaRegenRate = value["mManaRegenRate"].GetFloat();
            if (value.HasMember("mNeutralAttackManaGain"))
                mNeutralAttackManaGain = value["mNeutralAttackManaGain"].GetInt();
            /*if (value.HasMember("mFireSlashManaCost"))
                mFireSlashManaCost = value["mFireSlashManaCost"].GetInt();
            if (value.HasMember("mWaterSlashManaCost"))
                mWaterSlashManaCost = value["mWaterSlashManaCost"].GetInt();
            if (value.HasMember("mSteamBurstManaCost"))
                mSteamBurstManaCost = value["mSteamBurstManaCost"].GetInt();*/

            // Invulnerability
            if (value.HasMember("mInvulnerabilityDuration"))
                mInvulnerabilityDuration = value["mInvulnerabilityDuration"].GetFloat();
            if (value.HasMember("mHitStunDuration"))
                mHitStunDuration = value["mHitStunDuration"].GetFloat();

            // Throwable inventory
            /*if (value.HasMember("explosiveKunaiCount"))
                throwableInventory.explosiveKunaiCount = value["explosiveKunaiCount"].GetInt();
            if (value.HasMember("explosiveKunaiMax"))
                throwableInventory.explosiveKunaiMax = value["explosiveKunaiMax"].GetInt();*/

            // Checkpoint
            if (value.HasMember("checkpointX"))
                checkpointData.checkpointX = value["checkpointX"].GetFloat();
            if (value.HasMember("checkpointY"))
                checkpointData.checkpointY = value["checkpointY"].GetFloat();
            if (value.HasMember("hasCheckpoint"))
                checkpointData.hasCheckpoint = value["hasCheckpoint"].GetBool();

            // Combat state
            if (value.HasMember("elementComboWindow"))
                elementComboWindow = value["elementComboWindow"].GetFloat();

            // Deserialize attackStats
            if (value.HasMember("attackStats") && value["attackStats"].IsArray())
            {
                attackStats.clear();
                const rapidjson::Value& attacksArray = value["attackStats"];

                for (rapidjson::SizeType i = 0; i < attacksArray.Size(); i++)
                {
                    const rapidjson::Value& attackObj = attacksArray[i];
                    AttackStats attack;

                    if (attackObj.HasMember("AttackName"))
                        attack.AttackName = attackObj["AttackName"].GetString();
                    if (attackObj.HasMember("mDamageMultiplier"))
                        attack.mDamageMultiplier = attackObj["mDamageMultiplier"].GetFloat();
                    if (attackObj.HasMember("mAttackSpeedMultiplier"))
                        attack.mAttackSpeedMultiplier = attackObj["mAttackSpeedMultiplier"].GetFloat();
                    if (attackObj.HasMember("animationClipName"))
                        attack.animationClipName = attackObj["animationClipName"].GetString();
                    if (attackObj.HasMember("triggerColliderIndex"))
                        attack.triggerColliderIndex = attackObj["triggerColliderIndex"].GetInt();

                    // New fields
                    if (attackObj.HasMember("elementType"))
                        attack.elementType = static_cast<ElementType>(attackObj["elementType"].GetInt());
                    if (attackObj.HasMember("manaCost"))
                        attack.manaCost = attackObj["manaCost"].GetInt();
                    if (attackObj.HasMember("attackRange"))
                        attack.attackRange = attackObj["attackRange"].GetFloat();
                    if (attackObj.HasMember("attackArc"))
                        attack.attackArc = attackObj["attackArc"].GetFloat();
                    if (attackObj.HasMember("applyBurn"))
                        attack.applyBurn = attackObj["applyBurn"].GetBool();
                    if (attackObj.HasMember("applyStun"))
                        attack.applyStun = attackObj["applyStun"].GetBool();
                    if (attackObj.HasMember("effectDuration"))
                        attack.effectDuration = attackObj["effectDuration"].GetFloat();

                    // Initialize runtime data based on player stats
                    attack.attackCd = 1.f / mAttackSpeed;
                    attack.attackCdCurr = 0.f;
                    attack.attackIsInCoolDown = false;

                    attackStats.push_back(attack);
                }
            }

            // Initialize runtime data
            currAttackIndex = 0;
            animatorState = PS_Idle;
            //isAlive = true;
            isStunned = false;
            isInvulnerable = false;
            //stunedDuration = 0.f;
            stunedTimer = 0.f;
            //invulnerabilityTimer = 0.f;
            lastElementUsed = ElementType::None;
            elementComboTimer = 0.f;
            mHealthRegenDelayTimer = 0.f;
            mCanRegenHealth = true;
            hasShield = false;
            isShieldBroken = false;
        }

        // ===== HELPER METHODS =====

        //// Mana management
        //bool HasEnoughMana(int cost) const { return mMana >= cost; }
        //bool ConsumeMana(int cost)
        //{
        //    if (mMana >= cost) { mMana -= cost; return true; }
        //    return false;
        //}
        //void AddMana(int amount) { mMana = min(mMana + amount, mMaxMana); }

        //// Health management
        //void TakeDamage(int damage)
        //{
        //    if (combatState.isInvulnerable) return;

        //    int actualDamage = max(1, damage - mDefense);
        //    mHealth -= actualDamage;

        //    // Reset health regen delay
        //    mCanRegenHealth = false;
        //    mHealthRegenDelayTimer = mHealthRegenDelay;

        //    // Trigger invulnerability
        //    combatState.isInvulnerable = true;
        //    combatState.invulnerabilityTimer = mInvulnerabilityDuration;

        //    // Trigger hit-stun
        //    combatState.isStunned = true;
        //    combatState.stunedTimer = mHitStunDuration;

        //    if (mHealth <= 0)
        //    {
        //        mHealth = 0;
        //        combatState.isAlive = false;
        //    }
        //}

        //void Heal(int amount) { mHealth = min(mHealth + amount, mMaxHealth); }

        //// Throwable management
        ///*bool UseKunai()
        //{
        //    if (throwableInventory.explosiveKunaiCount > 0)
        //    {
        //        throwableInventory.explosiveKunaiCount--;
        //        return true;
        //    }
        //    return false;
        //}
        //void AddKunai(int count)
        //{
        //    throwableInventory.explosiveKunaiCount = min(
        //        throwableInventory.explosiveKunaiCount + count,
        //        throwableInventory.explosiveKunaiMax
        //    );
        //}*/

        //// Checkpoint management
        //void SetCheckpoint(float x, float y)
        //{
        //    checkpointData.checkpointX = x;
        //    checkpointData.checkpointY = y;
        //    checkpointData.hasCheckpoint = true;
        //}

        //// Elemental combo management
        //void SetLastElement(ElementType element)
        //{
        //    combatState.lastElementUsed = element;
        //    combatState.elementComboTimer = combatState.elementComboWindow;
        //}

        //bool CanTriggerSteamBurst() const
        //{
        //    return combatState.elementComboTimer > 0 &&
        //        combatState.lastElementUsed != ElementType::None;
        //}

        //// Respawn
        //void Respawn()
        //{
        //    mHealth = mMaxHealth;
        //    mMana = mMaxMana;
        //    combatState.isAlive = true;
        //    combatState.isStunned = false;
        //    combatState.isInvulnerable = true;
        //    combatState.invulnerabilityTimer = mInvulnerabilityDuration * 2;  // Extra i-frames on respawn
        //    combatState.lastElementUsed = ElementType::None;
        //    combatState.elementComboTimer = 0.f;
        //    animatorState = PS_Idle;
        //}

        //// Update timers (call from system update)
        //void UpdateTimers(float dt)
        //{
        //    //// Cooldowns
        //    //if (combatState.dash_cd_curr > 0)
        //    //    combatState.dash_cd_curr -= dt;
        //    //if (combatState.fire_slash_cd_curr > 0)
        //    //    combatState.fire_slash_cd_curr -= dt;
        //    //if (combatState.water_slash_cd_curr > 0)
        //    //    combatState.water_slash_cd_curr -= dt;
        //    //if (combatState.attack_1_cd_curr > 0)
        //    //    combatState.attack_1_cd_curr -= dt;
        //    //if (combatState.attack_2_cd_curr > 0)
        //    //    combatState.attack_2_cd_curr -= dt;

        //    //// Update cooldown flags
        //    //combatState.dash_is_in_cd = combatState.dash_cd_curr > 0;
        //    //combatState.fire_slash_is_in_cd = combatState.fire_slash_cd_curr > 0;
        //    //combatState.water_slash_is_in_cd = combatState.water_slash_cd_curr > 0;
        //    //combatState.attack_1_is_in_cd = combatState.attack_1_cd_curr > 0;
        //    //combatState.attack_2_is_in_cd = combatState.attack_2_cd_curr > 0;

        //    // Invulnerability
        //    if (combatState.isInvulnerable)
        //    {
        //        combatState.invulnerabilityTimer -= dt;
        //        if (combatState.invulnerabilityTimer <= 0)
        //            combatState.isInvulnerable = false;
        //    }

        //    // Stun
        //    if (combatState.isStunned)
        //    {
        //        combatState.stunedTimer -= dt;
        //        if (combatState.stunedTimer <= 0)
        //            combatState.isStunned = false;
        //    }

        //    // Health regen delay
        //    if (!mCanRegenHealth)
        //    {
        //        mHealthRegenDelayTimer -= dt;
        //        if (mHealthRegenDelayTimer <= 0)
        //            mCanRegenHealth = true;
        //    }

        //    // Health regeneration
        //    if (mCanRegenHealth && mHealth < mMaxHealth && mHealth > 0)
        //    {
        //        mHealth += static_cast<int>(mHealthRegenRate * dt);
        //        if (mHealth > mMaxHealth)
        //            mHealth = mMaxHealth;
        //    }

        //    // Mana regeneration (always active)
        //    if (mMana < mMaxMana)
        //    {
        //        mMana += static_cast<int>(mManaRegenRate * dt);
        //        if (mMana > mMaxMana)
        //            mMana = mMaxMana;
        //    }

        //    // Elemental combo timer
        //    if (combatState.elementComboTimer > 0)
        //    {
        //        combatState.elementComboTimer -= dt;
        //        if (combatState.elementComboTimer <= 0)
        //            combatState.lastElementUsed = ElementType::None;
        //    }
        //}
    };
}