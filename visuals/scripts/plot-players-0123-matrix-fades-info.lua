if type(window) ~= "userdata" then
  window = ofWindow()
end

local clock = ofClock(this, "setup")
local canvas = ofCanvas(this)
local fontDir = canvas:getDir() .. "/fonts/"
local arial = ofTrueTypeFont()
local fontSize = 14;


-- Pd [table] array is 500x300. using that as the default size
local winWidth = 1000 -- 1000
local winHeight = 600 -- 600


local array0 = ofArray("player-0-raw-output-plot")
local array1 = ofArray("player-1-raw-output-plot")
local array2 = ofArray("player-2-raw-output-plot")
local array3 = ofArray("player-3-raw-output-plot")

local player0brightness = ofValue("player-0-draw-brightness")
local player1brightness = ofValue("player-1-draw-brightness")
local player2brightness = ofValue("player-2-draw-brightness")
local player3brightness = ofValue("player-3-draw-brightness")

local showData = ofValue("visuals-show-data-ofValue")
local xPos = ofValue("visuals-xpos-ofValue")

local player0gain = ofValue("player-0-gain-ofValue")
local player0algo = ofValue("player-0-algo-select-ofValue")
local player0bitDepth = ofValue("player-0-bit-depth-ofValue")
local player0tempo = ofValue("player-0-tempo-final-ofValue")

local player1gain = ofValue("player-1-gain-ofValue")
local player1algo = ofValue("player-1-algo-select-ofValue")
local player1bitDepth = ofValue("player-1-bit-depth-ofValue")
local player1tempo = ofValue("player-1-tempo-final-ofValue")

local player2gain = ofValue("player-2-gain-ofValue")
local player2algo = ofValue("player-2-algo-select-ofValue")
local player2bitDepth = ofValue("player-2-bit-depth-ofValue")
local player2tempo = ofValue("player-2-tempo-final-ofValue")

local player3gain = ofValue("player-3-gain-ofValue")
local player3algo = ofValue("player-3-algo-select-ofValue")
local player3bitDepth = ofValue("player-3-bit-depth-ofValue")
local player3tempo = ofValue("player-3-tempo-final-ofValue")

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
  window:setPosition(xPos:get(), 100)
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
  arial:load(fontDir .. "Arial.ttf", fontSize)
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
	local halfWinHeight = winHeight * 0.5
	local leftMargin = 10
	local topMargin = 10
	local lineSpacing = 1.75
	
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

	if showData:get() > 0 then
	
		-- draw player info
		ofSetColor(75, 189, 75)
	
		local p0algoString = string.format("algo: %1.0f", player0algo:get())
		local p0gainString = string.format("gain: %1.0f", player0gain:get())
		local p0bitDepthString = string.format("bit: %1.0f", player0bitDepth:get())
		local p0tempoString = string.format("tempo: %0.2f", player0tempo:get())
	
		local p1algoString = string.format("algo: %1.0f", player1algo:get())
		local p1gainString = string.format("gain: %1.0f", player1gain:get())
		local p1bitDepthString = string.format("bit: %1.0f", player1bitDepth:get())
		local p1tempoString = string.format("tempo: %0.2f", player1tempo:get())

		local p2algoString = string.format("algo: %1.0f", player2algo:get())
		local p2gainString = string.format("gain: %1.0f", player2gain:get())
		local p2bitDepthString = string.format("bit: %1.0f", player2bitDepth:get())
		local p2tempoString = string.format("tempo: %0.2f", player2tempo:get())

		local p3algoString = string.format("algo: %1.0f", player3algo:get())
		local p3gainString = string.format("gain: %1.0f", player3gain:get())
		local p3bitDepthString = string.format("bit: %1.0f", player3bitDepth:get())
		local p3tempoString = string.format("tempo: %0.2f", player3tempo:get())
	
		arial:drawString(p0algoString, leftMargin, fontSize * lineSpacing * 1)
		arial:drawString(p0gainString, leftMargin, fontSize * lineSpacing * 2)
		arial:drawString(p0tempoString, leftMargin + winWidth * 0.16667, fontSize * lineSpacing * 1)
		arial:drawString(p0bitDepthString, leftMargin + winWidth * 0.16667, fontSize * lineSpacing * 2)

		arial:drawString(p1algoString, leftMargin + halfWinWidth, fontSize * lineSpacing * 1)
		arial:drawString(p1gainString, leftMargin + halfWinWidth, fontSize * lineSpacing * 2)
		arial:drawString(p1tempoString, leftMargin + winWidth * 0.66667, fontSize * lineSpacing * 1)
		arial:drawString(p1bitDepthString, leftMargin + winWidth * 0.66667, fontSize * lineSpacing * 2)
	
		arial:drawString(p2algoString, leftMargin, halfWinHeight + fontSize * lineSpacing * 1)
		arial:drawString(p2gainString, leftMargin, halfWinHeight + fontSize * lineSpacing * 2)
		arial:drawString(p2tempoString, leftMargin + winWidth * 0.16667, halfWinHeight + fontSize * lineSpacing * 1)
		arial:drawString(p2bitDepthString, leftMargin + winWidth * 0.16667, halfWinHeight + fontSize * lineSpacing * 2)

		arial:drawString(p3algoString, leftMargin + halfWinWidth, halfWinHeight + fontSize * lineSpacing * 1)
		arial:drawString(p3gainString, leftMargin + halfWinWidth, halfWinHeight + fontSize * lineSpacing * 2)
		arial:drawString(p3tempoString, leftMargin + winWidth * 0.66667, halfWinHeight + fontSize * lineSpacing * 1)
		arial:drawString(p3bitDepthString, leftMargin + winWidth * 0.66667, halfWinHeight + fontSize * lineSpacing * 2)
  	
  	end
  	
end

















