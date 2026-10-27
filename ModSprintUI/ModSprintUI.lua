-- Original client addon for mod-sprint-wotlk (WoW 3.3.5a).
-- No changes to keybindings are made without explicit player consent.
local addon = CreateFrame("Frame", "ModSprintUIFrame", UIParent)
addon:RegisterEvent("PLAYER_LOGIN")
addon:RegisterEvent("CHAT_MSG_SYSTEM")
addon:RegisterEvent("CHAT_MSG_ADDON")
-- Hide only the server's sprint telemetry, while retaining the event for the meter.
ChatFrame_AddMessageEventFilter("CHAT_MSG_SYSTEM", function(self, event, message)
    if type(message) == "string" and string.find(message, "^AC_SPRINT:%d+%s*$") then
        return true
    end
    return false
end)
-- Some 3.3.5a clients route server system text through chat frames
-- without honoring CHAT_MSG_SYSTEM filters. Filter the final display too.
local function IsSprintTelemetry(message)
    return type(message) == "string"
        and string.find(message, "AC_SPRINT:%d+") ~= nil
end

local function InstallChatFrameFilter(frame)
    if not frame or frame.ModSprintOriginalAddMessage then return end
    local original = frame.AddMessage
    if type(original) ~= "function" then return end
    frame.ModSprintOriginalAddMessage = original
    frame.AddMessage = function(self, message, ...)
        if IsSprintTelemetry(message) then return end
        return original(self, message, ...)
    end
end

for i = 1, (NUM_CHAT_WINDOWS or 10) do
    InstallChatFrameFilter(_G["ChatFrame" .. i])
end
InstallChatFrameFilter(DEFAULT_CHAT_FRAME)

addon:RegisterEvent("MODIFIER_STATE_CHANGED")
addon:RegisterEvent("PLAYER_REGEN_ENABLED")
local db, bar, fill, label
local active = false
local lastValue = 100

local function SayCommand(cmd)
    SendChatMessage(cmd, "SAY")
end

local function SprintStop()
    if active then
        active = false
        SayCommand(".sprintstop")
    end
end

local function SprintStart()
    if not active then
        active = true
        SayCommand(".sprintstart")
    end
end

local function SprintKey()
    return db and db.key or "SHIFT"
end

local function KeyIsDown(key)
    if key == "SHIFT" then return IsShiftKeyDown() end
    if key == "CTRL" then return IsControlKeyDown() end
    if key == "ALT" then return IsAltKeyDown() end
    return false
end

local function CheckSprintState()
    if not db then return end
    if KeyIsDown(SprintKey()) then SprintStart() else SprintStop() end
end

local function UpdateMeter(percent)
    percent = math.max(0, math.min(100, percent))
    lastValue = percent
    if fill then fill:SetWidth(196 * percent / 100) end
    if label then label:SetText("Sprint: " .. percent .. "%") end
end

local function BindingConflict()
    if not db or db.key ~= "SHIFT" then return nil end
    local action = GetBindingAction("SHIFT-SPACE")
    if action and action ~= "" then return action end
    return nil
end

local function RepairBinding()
    if InCombatLockdown() then
        db.pendingRepair = true
        DEFAULT_CHAT_FRAME:AddMessage("|cffffcc00[Sprint]|r Binding fix will run when combat ends.")
        return
    end
    local action = BindingConflict()
    if not action then
        db.pendingRepair = false
        return
    end
    -- Only unbind the conflicting combination; SPACE remains untouched.
    if SetBinding("SHIFT-SPACE") then
        SaveBindings(GetCurrentBindingSet())
        db.pendingRepair = false
        DEFAULT_CHAT_FRAME:AddMessage("|cff00ff00[Sprint]|r Removed SHIFT-SPACE binding (" .. action .. "). SPACE remains unchanged.")
    else
        DEFAULT_CHAT_FRAME:AddMessage("|cffff4444[Sprint]|r Could not clear SHIFT-SPACE binding.")
    end
end

StaticPopupDialogs["MODSPRINT_BINDING_FIX"] = {
    text = "Sprint uses Shift, but SHIFT-SPACE is currently assigned to %s. Unbind only SHIFT-SPACE so you can jump while sprinting? Your regular SPACE binding will not change.",
    button1 = YES,
    button2 = NO,
    OnAccept = function()
        db.bindingChoice = "accepted"
        RepairBinding()
    end,
    OnCancel = function()
        db.bindingChoice = "declined"
    end,
    timeout = 0,
    whileDead = 1,
    hideOnEscape = 1,
    preferredIndex = 3
}

local function CheckBinding()
    if not db or db.key ~= "SHIFT" then return end
    local conflict = BindingConflict()
    if conflict and not db.bindingChoice and not InCombatLockdown() then
        StaticPopup_Show("MODSPRINT_BINDING_FIX", conflict)
    end
end

local function MakeBar()
    bar = CreateFrame("Frame", "ModSprintMeter", UIParent)
    bar:SetWidth(200)
    bar:SetHeight(18)
    bar:SetPoint(db.point or "CENTER", UIParent, db.point or "CENTER", db.x or 0, db.y or -200)
    bar:SetMovable(true)
    bar:EnableMouse(true)
    bar:RegisterForDrag("LeftButton")
    local bg = bar:CreateTexture(nil, "BACKGROUND")
    bg:SetAllPoints(bar)
    bg:SetTexture(0, 0, 0, 0.75)
    fill = bar:CreateTexture(nil, "ARTWORK")
    fill:SetPoint("TOPLEFT", bar, "TOPLEFT", 2, -2)
    fill:SetWidth(196)
    fill:SetHeight(14)
    fill:SetTexture(0.2, 0.65, 1, 1)
    label = bar:CreateFontString(nil, "OVERLAY", "GameFontHighlightSmall")
    label:SetPoint("CENTER", bar)
    bar:SetScript("OnDragStart", function(self)
        if not db.locked then self:StartMoving() end
    end)
    bar:SetScript("OnDragStop", function(self)
        self:StopMovingOrSizing()
        local point, _, _, x, y = self:GetPoint(1)
        db.point, db.x, db.y = point, x, y
    end)
    UpdateMeter(lastValue)
end

SLASH_MODSPRINTBIND1 = "/sprintbind"
SlashCmdList["MODSPRINTBIND"] = function(msg)
    if not db then return end
    msg = string.upper(string.gsub(msg or "", "^%s*(.-)%s*$", "%1"))
    if msg == "" then
        DEFAULT_CHAT_FRAME:AddMessage("|cff33ccff[Sprint]|r Current key: " .. db.key .. ". Usage: /sprintbind SHIFT, CTRL, or ALT")
        return
    end
    if msg ~= "SHIFT" and msg ~= "CTRL" and msg ~= "ALT" then
        DEFAULT_CHAT_FRAME:AddMessage("|cffff4444[Sprint]|r This version supports SHIFT, CTRL and ALT only.")
        return
    end
    SprintStop()
    db.key = msg
    db.bindingChoice = nil
    DEFAULT_CHAT_FRAME:AddMessage("|cff33ccff[Sprint]|r Sprint key set to " .. msg)
    CheckBinding()
end

SLASH_MODSPRINTLOCK1 = "/sprintlock"
SlashCmdList["MODSPRINTLOCK"] = function()
    if not db then return end
    db.locked = not db.locked
    DEFAULT_CHAT_FRAME:AddMessage("|cff33ccff[Sprint]|r Meter " .. (db.locked and "locked" or "unlocked; drag to move") .. ".")
end

SLASH_MODSPRINTFIX1 = "/sprintfixjump"
SlashCmdList["MODSPRINTFIX"] = function()
    if not db then return end
    db.bindingChoice = nil
    local conflict = BindingConflict()
    if conflict then
        if InCombatLockdown() then
            DEFAULT_CHAT_FRAME:AddMessage("|cffffcc00[Sprint]|r Try again after combat.")
        else
            StaticPopup_Show("MODSPRINT_BINDING_FIX", conflict)
        end
    else
        DEFAULT_CHAT_FRAME:AddMessage("|cff33ccff[Sprint]|r No SHIFT-SPACE conflict found.")
    end
end

addon:SetScript("OnEvent", function(self, event, ...)
    if event == "PLAYER_LOGIN" then
        if not ModSprintDB then ModSprintDB = {} end
        db = ModSprintDB
        if not db.key then db.key = "SHIFT" end
        if db.locked == nil then db.locked = true end
        MakeBar()
        CheckBinding()
    elseif event == "CHAT_MSG_ADDON" then
        local prefix, payload, distribution, sender = ...
        if prefix == "ACSPRINT" and distribution == "WHISPER" then
            local pct = tonumber(payload)
            if pct then UpdateMeter(pct) end
        end
    elseif event == "CHAT_MSG_SYSTEM" then
        local message = ...
        local pct = message and string.match(message, "AC_SPRINT:(%d+)")
        if pct then UpdateMeter(tonumber(pct)) end
    elseif event == "MODIFIER_STATE_CHANGED" then
        CheckSprintState()
    elseif event == "PLAYER_REGEN_ENABLED" then
        if db and db.pendingRepair then RepairBinding() end
        CheckBinding()
    end
end)

-- Periodically reconcile missed key-up events, e.g. alt-tab while holding Shift.
local timer = 0
addon:SetScript("OnUpdate", function(self, elapsed)
    if not db then return end
    timer = timer + elapsed
    if timer >= 0.15 then
        timer = 0
        CheckSprintState()
    end
end)
