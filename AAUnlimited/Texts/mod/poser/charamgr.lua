local _M = {}

local signals = require "poser.signals"

local dummycharacter = {}
local dummymt = {
	__call = function()
		log.warn("calling dummy character")
		return nil
	end,
	
	__index = function()
		log.warn("indexing dummy character")
		return dummycharacter
	end,
	
	__pairs = function()
		log.warn("iterating dummy character")
		return nil
	end
}
setmetatable(dummycharacter, dummymt)
_M.current = dummycharacter

local characters = {}
local currentcharacterchanged = signals.signal()
local characterschanged = signals.signal()

-- Character Helper Functions
local function GetSlider(character, slider)
	return character.poser:GetSlider(slider)
end
local function Sliders(character)
	local poser = character.poser
	return poser:Sliders()
end
local function Props(character)
	local poser = character.poser
	return poser:Props()
end
local function SetClip(character, clip)
	local skel = character.struct.m_xxSkeleton
	skel.m_poseNumber = clip
	skel.m_animFrame = 9000
end
local function GetXXFileFace(character)
	return character.struct:GetXXFileFace()
end

-- Character Helper Metatables
local facekeys = { eye = "m_eye", eyeopen = "m_eyeOpen", eyebrow = "m_eyebrow", mouth = "m_mouth", mouthopen = "m_mouthOpen", blush = "Blush", blushlines = "BlushLines" }
local skelkeys = { pose = "m_poseNumber", frame = "m_animFrame", skelname = "m_name" }
local charkeys = { clothstate = "m_clothState", spawn = "Spawn", }
local charamt = {}
charamt.GetSlider = GetSlider
charamt.Sliders = Sliders
charamt.Props = Props
charamt.SetClip = SetClip
charamt.GetXXFileFace = GetXXFileFace
charamt.isvalid = true
charamt.override = function(character, skeleton, override)
	character.poser:Override(skeleton, override)
end

function charamt.despawn(char)
	if not char.spawned then return false end
	if char == _M.current then
		_M.setcurrentcharacter(characters[1] or dummycharacter)
	end
	char.struct:Despawn()
	char.spawn = false
	return true
end

function charamt.update(char, clothstate)
	char.struct:Update(clothstate or 0, exe_type == "edit" and 1 or 0)
end

function charamt.skeleton(char, pose)
	return _M.skeleton(char.struct, pose)
end

function charamt.reload(char, light)
	-- TODO: light/partial now hardcoded
	char:spawn(1,char.index, light, exe_type == "edit" and 1 or 0)
	char:skeleton()
end

function charamt.update_face(chara)
--	local blink = exe_type == "edit" or 0xFC720 or 0x10DBB0
--	local saved = g_poke(blink, "\xc2\x08\x00")
	--chara.struct:Animate2(-1,0,1,1,1,1)
	--chara.struct:Animate1(0,1,0)
	--saved = g_poke(blink, saved)
end

function charamt.__index(character,k)
	if facekeys[k] then
		local face = character.struct:GetXXFileFace()
		return face[facekeys[k]]
	end
	if skelkeys[k] then
		local skel = character.struct.m_xxSkeleton
		return skel[skelkeys[k]]
	end
	if charkeys[k] then
		return character.struct[charkeys[k]]
	end
	if charamt[k] then
		return charamt[k]
	end
end
function charamt.__newindex(character,k,v)
	if facekeys[k] then
		local face = character.struct:GetXXFileFace()
		face[facekeys[k]] = v
		character:update_face()
	elseif skelkeys[k] then
		local skel = character.struct.m_xxSkeleton
		skel[skelkeys[k]] = v
	end
end

local function setcurrentcharacter(character)
	log.spam("Poser: Set current character %s", character)
	if type(character) == "number" then
		_M.current = characters[character]
		currentcharacterchanged(_M.current)
	elseif _M.current ~= character then
		_M.current = character
		currentcharacterchanged(_M.current)
	end
end

local charcount = 0 -- this counts only characters spawned by the game, not us
function _M.addcharacter(character)
	log.spam("Poser: Add character %s", character)
	local new = true
	local last = 0
	for i,v in ipairs(characters) do
		if v.struct == character then
			new = false
		end
		last = i
	end
	if new then
		local data = character.m_charData
		local name = sjis_to_utf8(string.format("%s %s", data.m_forename, data.m_surname))
		if not _M.is_spawning then
			charcount = charcount + 1
		end
		new = { index = last, name = name, struct = character, poser = GetPoserCharacter(character), data = data, GetSlider = GetSlider, spawned = _M.is_spawning }
		setmetatable(new, charamt)
		characters[last + 1] = new
		if _M.current ~= dummycharacter then
			setcurrentcharacter(new)
		end
	end
	log.spam("Poser: We have %d characters", #characters)
	characterschanged()
end

function _M.removecharacter(character)
	log.spam("Poser: Remove character %s", character)
	if _M.current and _M.current.struct == character then
		setcurrentcharacter(dummycharacter)
	end
	for k,v in pairs(characters) do
		if v.struct == character then
			table.remove(characters, k)
			break
		end
	end
	log.spam("Poser: We have %d characters", #characters)
	characterschanged()
	if character.spawned then return end
	charcount = charcount - 1
	-- the legit character despawned, this means the scene is ending - despawn all our injected characters, too
	if charcount == 0 then
		while #characters > 0 do
			-- all characters remaining must be spawned ones
			assert(characters[1].spawned)
			characters[1].struct:Despawn()
		end
	end
end

function _M.spawn(seat,cloth,pose)
	_M.is_spawning = true
	local ch = GetCharacter(seat)
	assert(ch)
	ch:Spawn(cloth or 1,#characters,0,0)
	ch:SetAnimData(-1,0,1,1,1,1,0,1,0)
	_M.skeleton(ch, pose)
	_M.is_spawning = false
end

function _M.skeleton(char, pose)
	local pp = host_path("data", "jg2e01_00_00.pp")
	-- TODO: xa name depends on some things (editor/play, gender ..), hardcoded for female clothing screen for now
	local xa = host_path("data", "HAE00_00_00_00.xa")
	char:Skeleton(pp, xa, pose or 1)
end


_M.characters = characters
_M.setcurrentcharacter = setcurrentcharacter
_M.currentcharacterchanged = currentcharacterchanged
_M.characterschanged = characterschanged
_M.is_spawning = false

return _M