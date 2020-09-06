if type(window) ~= "userdata" then
  window = ofWindow()
end

local xPos = ofValue("visuals-xpos-ofValue")
local winScale = ofValue("visuals-winScale-ofValue")

local clock = ofClock(this, "setup")
local canvas = ofCanvas(this)
local fontDir = canvas:getDir() .. "/fonts/"
local arial = ofTrueTypeFont()
local fontSize = 14 * winScale:get();


-- Pd [table] array is 500x300. using that to determine aspect ratio
local winWidth = math.floor(1000 * winScale:get() + 0.5) -- 1000
local winHeight = math.floor(600 * winScale:get() + 0.5) -- 600

local halfWinWidth = winWidth * 0.5

local timerMin = ofValue("timer-min-ofValue")
local timerSec = ofValue("timer-sec-ofValue")

local array0 = ofArray("player-0-raw-output-plot")
local array1 = ofArray("player-1-raw-output-plot")
local array2 = ofArray("player-2-raw-output-plot")
local array3 = ofArray("player-3-raw-output-plot")

local player0brightness = ofValue("player-0-draw-brightness")
local player1brightness = ofValue("player-1-draw-brightness")
local player2brightness = ofValue("player-2-draw-brightness")
local player3brightness = ofValue("player-3-draw-brightness")

local showData = ofValue("visuals-show-data-ofValue")

local player0gain = ofValue("player-0-gain-ofValue")
local player0algo = ofValue("player-0-algo-select-ofValue")
local player0bitDepth = ofValue("player-0-bit-depth-ofValue")
local player0tempo = ofValue("player-0-tempo-final-ofValue")
local player0dryWet = ofValue("player-0-dry-wet-ofValue")
local player0liveness = ofValue("player-0-rvb-liveness-ofValue")

local player1gain = ofValue("player-1-gain-ofValue")
local player1algo = ofValue("player-1-algo-select-ofValue")
local player1bitDepth = ofValue("player-1-bit-depth-ofValue")
local player1tempo = ofValue("player-1-tempo-final-ofValue")
local player1dryWet = ofValue("player-1-dry-wet-ofValue")
local player1liveness = ofValue("player-1-rvb-liveness-ofValue")

local player2gain = ofValue("player-2-gain-ofValue")
local player2algo = ofValue("player-2-algo-select-ofValue")
local player2bitDepth = ofValue("player-2-bit-depth-ofValue")
local player2tempo = ofValue("player-2-tempo-final-ofValue")
local player2dryWet = ofValue("player-2-dry-wet-ofValue")
local player2liveness = ofValue("player-2-rvb-liveness-ofValue")

local player3gain = ofValue("player-3-gain-ofValue")
local player3algo = ofValue("player-3-algo-select-ofValue")
local player3bitDepth = ofValue("player-3-bit-depth-ofValue")
local player3tempo = ofValue("player-3-tempo-final-ofValue")
local player3dryWet = ofValue("player-3-dry-wet-ofValue")
local player3liveness = ofValue("player-3-rvb-liveness-ofValue")

-- only need to check the size of array0 since they're all the same
local arraySize = array0:getSize()
local tableIncr = arraySize/halfWinWidth

-- declare data structures?
-- size of each array is the same as the width of the window. we'll draw one value per pixel
M.num = halfWinWidth
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

	local leftMargin = 10
	local topMargin = 10
	local lineSpacing = 1.75

	ofSetLineWidth(3)

	for i=0, winWidth-1 do
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

        local timerString = string.format("%1.0f : %1.0f", timerMin:get(), timerSec:get())

		local p0algoString = string.format("algo: %1.0f", player0algo:get())
		local p0gainString = string.format("gain: %1.0f", player0gain:get())
		local p0bitDepthString = string.format("bit: %1.0f", player0bitDepth:get())
		local p0tempoString = string.format("tempo: %0.2f", player0tempo:get())
		local p0dryWetString = string.format("dry-wet: %1.0f", player0dryWet:get())
		local p0livenessString = string.format("liveness: %1.0f", player0liveness:get())

		local p1algoString = string.format("algo: %1.0f", player1algo:get())
		local p1gainString = string.format("gain: %1.0f", player1gain:get())
		local p1bitDepthString = string.format("bit: %1.0f", player1bitDepth:get())
		local p1tempoString = string.format("tempo: %0.2f", player1tempo:get())
		local p1dryWetString = string.format("dry-wet: %1.0f", player1dryWet:get())
		local p1livenessString = string.format("liveness: %1.0f", player1liveness:get())

		local p2algoString = string.format("algo: %1.0f", player2algo:get())
		local p2gainString = string.format("gain: %1.0f", player2gain:get())
		local p2bitDepthString = string.format("bit: %1.0f", player2bitDepth:get())
		local p2tempoString = string.format("tempo: %0.2f", player2tempo:get())
		local p2dryWetString = string.format("dry-wet: %1.0f", player2dryWet:get())
		local p2livenessString = string.format("liveness: %1.0f", player2liveness:get())

		local p3algoString = string.format("algo: %1.0f", player3algo:get())
		local p3gainString = string.format("gain: %1.0f", player3gain:get())
		local p3bitDepthString = string.format("bit: %1.0f", player3bitDepth:get())
		local p3tempoString = string.format("tempo: %0.2f", player3tempo:get())
		local p3dryWetString = string.format("dry-wet: %1.0f", player3dryWet:get())
		local p3livenessString = string.format("liveness: %1.0f", player3liveness:get())

        -- make the timer yellow, half opacity
		ofSetColor(255, 255, 0, 128)
        arial:drawString(timerString, halfWinWidth - (leftMargin*2), winHeight * 0.5 + (topMargin * 0.5))

        -- other data greenish, full opacity
		ofSetColor(75, 189, 75)
		arial:drawString(p0algoString, leftMargin, fontSize * lineSpacing * 1)
		arial:drawString(p0gainString, leftMargin, fontSize * lineSpacing * 2)
		arial:drawString(p0tempoString, leftMargin + winWidth * 0.1, fontSize * lineSpacing * 1)
		arial:drawString(p0bitDepthString, leftMargin + winWidth * 0.1, fontSize * lineSpacing * 2)
		arial:drawString(p0dryWetString, leftMargin + winWidth * 0.25, fontSize * lineSpacing * 1)
		arial:drawString(p0livenessString, leftMargin + winWidth * 0.25, fontSize * lineSpacing * 2)

		arial:drawString(p1algoString, leftMargin + halfWinWidth, fontSize * lineSpacing * 1)
		arial:drawString(p1gainString, leftMargin + halfWinWidth, fontSize * lineSpacing * 2)
		arial:drawString(p1tempoString, leftMargin + winWidth * 0.6, fontSize * lineSpacing * 1)
		arial:drawString(p1bitDepthString, leftMargin + winWidth * 0.6, fontSize * lineSpacing * 2)
		arial:drawString(p1dryWetString, leftMargin + winWidth * 0.75, fontSize * lineSpacing * 1)
		arial:drawString(p1livenessString, leftMargin + winWidth * 0.75, fontSize * lineSpacing * 2)

		arial:drawString(p2algoString, leftMargin, winHeight - fontSize * lineSpacing * 2)
		arial:drawString(p2gainString, leftMargin, winHeight - fontSize * lineSpacing * 1)
		arial:drawString(p2tempoString, leftMargin + winWidth * 0.1, winHeight - fontSize * lineSpacing * 2)
		arial:drawString(p2bitDepthString, leftMargin + winWidth * 0.1, winHeight - fontSize * lineSpacing * 1)
		arial:drawString(p2dryWetString, leftMargin + winWidth * 0.25, winHeight - fontSize * lineSpacing * 2)
		arial:drawString(p2livenessString, leftMargin + winWidth * 0.25, winHeight - fontSize * lineSpacing * 1)

		arial:drawString(p3algoString, leftMargin + halfWinWidth, winHeight - fontSize * lineSpacing * 2)
		arial:drawString(p3gainString, leftMargin + halfWinWidth, winHeight - fontSize * lineSpacing * 1)
		arial:drawString(p3tempoString, leftMargin + winWidth * 0.6, winHeight - fontSize * lineSpacing * 2)
		arial:drawString(p3bitDepthString, leftMargin + winWidth * 0.6, winHeight - fontSize * lineSpacing * 1)
		arial:drawString(p3dryWetString, leftMargin + winWidth * 0.75, winHeight - fontSize * lineSpacing * 2)
		arial:drawString(p3livenessString, leftMargin + winWidth * 0.75, winHeight - fontSize * lineSpacing * 1)

  	end

end
