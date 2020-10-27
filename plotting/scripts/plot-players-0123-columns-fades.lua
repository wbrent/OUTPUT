if type(window) ~= "userdata" then
  window = ofWindow()
end

local clock = ofClock(this, "setup")

-- Pd [table] array is 500x300. using that as the default size
local winWidth = 1000
local winHeight = 600

local array0 = ofArray("player-0-raw-output-plot")
local array1 = ofArray("player-1-raw-output-plot")
local array2 = ofArray("player-2-raw-output-plot")
local array3 = ofArray("player-3-raw-output-plot")

local player0brightness = ofValue("player-0-draw-brightness")
local player1brightness = ofValue("player-1-draw-brightness")
local player2brightness = ofValue("player-2-draw-brightness")
local player3brightness = ofValue("player-3-draw-brightness")

-- only need to check the size of array0 since they're all the same
local arraySize = array0:getSize()
local tableIncr = arraySize/winHeight

-- declare data structures?
-- size of each array is the same as the width of the window. we'll draw one value per pixel
M.num = winHeight
M.a0, M.a1, M.a2, M.a3 = ofTable(), ofTable(), ofTable(), ofTable()

-- window setup constructor
function M.new()
  ofWindow.addListener("setup", this)
  ofWindow.addListener("update", this)
  ofWindow.addListener("draw", this)
  window:setPosition(100, 100)
  window:setSize(winWidth, winHeight)
  if ofWindow.exists then
    clock:delay(0)
  else
    window:create()
  end
end

-- destructor
function M.free()
  window:destroy()
  ofWindow.removeListener("setup", this)
  ofWindow.removeListener("update", this)
  ofWindow.removeListener("draw", this)
end

-- framerate, background color, etc
function M.setup()
  ofSetWindowTitle("players 0123")
  ofSetFrameRate(40)
  ofBackground(0, 0, 0, 255)
--  ofDisableSmoothing()
end

-- translate the signal data to pixels relative to the window height
function M.update()

	for i=0, M.num do
		-- floor to the nearest valid Pd array index using i and tableIncr
		local arrayIdx = math.floor(i*tableIncr + 0.5)
		-- set a scale variable relative to window width
		local ampScale = 0.1 * winWidth -- 0.125 is full range
		-- get the value in the table, multiply by ampScale
		-- 
		-- 
		M.a0[i] = (array0:getAt(arrayIdx) * ampScale) + (0.125*winWidth)
		M.a1[i] = (array1:getAt(arrayIdx) * ampScale) + (0.375*winWidth)
		
		-- 
		M.a2[i] = (array2:getAt(arrayIdx) * ampScale) + (0.625*winWidth)
		M.a3[i] = (array3:getAt(arrayIdx) * ampScale) + (0.875*winWidth)
	end
	
end

function M.draw()
		
	ofSetLineWidth(3)

	for i=0, M.num-1 do
		local brightness
		
		brightness = player0brightness:get()
		ofSetColor(brightness, brightness, brightness)
		ofDrawLine(M.a0[i], i, M.a0[i+1], i+1)
		
		brightness = player2brightness:get()
		ofSetColor(brightness, brightness, brightness)
		ofDrawLine(M.a2[i], i, M.a2[i+1], i+1)

		brightness = player1brightness:get()
		ofSetColor(brightness, brightness, brightness)
		ofDrawLine(M.a1[i], i, M.a1[i+1], i+1)

		brightness = player3brightness:get()
		ofSetColor(brightness, brightness, brightness)
		ofDrawLine(M.a3[i], i, M.a3[i+1], i+1)

	end

end