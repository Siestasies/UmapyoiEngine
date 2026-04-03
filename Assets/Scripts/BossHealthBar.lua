local originalPosX
local originalScaleX
local bhp = require("BossHPState")
local img

function Start()
    img = GetImage()
end
  
function Update(dt)
    local currentHealth = bhp.GetBossHP()
    local maxHealth = 100

    local ratio = currentHealth/maxHealth * 0.7
    img.fillAmount = ratio + 0.15

    -- if needed in your engine:
    -- SetTransform(transform)
end
