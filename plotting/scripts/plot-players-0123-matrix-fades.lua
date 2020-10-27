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
local tableIncr = arraySize/winWidth

-- declare data structures?
-- size of each array is the same as the width of the window. we'll draw one value per pixel
M.num = winWidth
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
		-- set a scale variable relative to window height
		local ampScale = 0.2 * winHeight -- 0.25 is full range
		-- get the value in the table, multiply by ampScale
		-- using -1 to flip the vertical axis, which draws relative to top
		-- offset by a quarter the window height for 01
		M.a0[i] = (-1 * array0:getAt(arrayIdx) * ampScale) + (0.25*winHeight)
		M.a1[i] = (-1 * array1:getAt(arrayIdx) * ampScale) + (0.25*winHeight)
		
		-- on the second row, plots 2 and 3 must be offset by 0.75 the window height
		M.a2[i] = (-1 * array2:getAt(arrayIdx) * ampScale) + (0.75*winHeight)
		M.a3[i] = (-1 * array3:getAt(arrayIdx) * ampScale) + (0.75*winHeight)
	end
	
end

function M.draw()
	
	local halfWinWidth = winWidth * 0.5
	
	ofSetLineWidth(3)

	for i=0, M.num-1 do
		local brightness
		
		if i < halfWinWidth then
			brightness = player0brightness:get()
			ofSetColor(brightness, brightness, brightness)
			ofDrawLine(i, M.a0[i], i+1, M.a0[i+1])
			
			brightness = player2brightness:get()
			ofSetColor(brightness, brightness, brightness)
			ofDrawLine(i, M.a2[i], i+1, M.a2[i+1])
		else
			local thisY = i - halfWinWidth
			
			brightness = player1brightness:get()
			ofSetColor(brightness, brightness, brightness)
			ofDrawLine(i, M.a1[thisY], i+1, M.a1[thisY+1])

			brightness = player3brightness:get()
			ofSetColor(brightness, brightness, brightness)
			ofDrawLine(i, M.a3[thisY], i+1, M.a3[thisY+1])
		end
		
	end

end