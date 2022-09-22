-- Copyright (c) 2014-2022, Asensing Group
--
-- Asensing LiDAR protocol plugin for Wireshark
--
-- Change Logs:
-- Date           Author       Notes
-- 2022-09-13     luhuadong    the first version


-- Declare our protocol
lidar_proto = Proto("Asensing","Asensing LiDAR")

-- Create a function to dissect it
function lidar_proto.dissector(buffer,pinfo,tree)
	pinfo.cols.protocol = "Asensing"
	local subtree = tree:add(lidar_proto,buffer(),"Asensing LiDAR packet data")

	local curr = 0

	local headerSize = 40

	local blockPerPacket = 12
	
	local nbLaser = 8
	local laserSize = 9
	local blockSize =  4 + nbLaser * laserSize
	
	local tailSize = 4

	-- Packet Header --
	local header_subtree = subtree:add(buffer(curr, headerSize), "Header")

	local Sob = buffer(curr, 4):uint()
	header_subtree:add(buffer(curr, 4), "Sob : " .. Sob)
	curr = curr + 4

	local FrameID = buffer(curr, 4):uint()
	header_subtree:add(buffer(curr, 4), "FrameID : " .. FrameID)
	curr = curr + 4

	local SeqNum = buffer(curr, 2):uint()
	header_subtree:add(buffer(curr, 2), "SeqNum : " .. SeqNum)
	curr = curr + 2

	local PkgLen = buffer(curr, 2):uint()
	header_subtree:add(buffer(curr, 2), "PkgLen : " .. PkgLen)
	curr = curr + 2

	local LidarType = buffer(curr, 2):uint()
	header_subtree:add(buffer(curr, 2), "LidarType : " .. LidarType)
	curr = curr + 2

	local VersionMajor = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "VersionMajor : " .. VersionMajor)
	curr = curr + 1

	local VersionMinor = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "VersionMinor : " .. VersionMinor)
	curr = curr + 1

	local UTCTime0 = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "UTCTime0 : " .. UTCTime0)
	curr = curr + 1

	local UTCTime1 = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "UTCTime1 : " .. UTCTime1)
	curr = curr + 1

	local UTCTime2 = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "UTCTime2 : " .. UTCTime2)
	curr = curr + 1

	local UTCTime3 = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "UTCTime3 : " .. UTCTime3)
	curr = curr + 1

	local UTCTime4 = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "UTCTime4 : " .. UTCTime4)
	curr = curr + 1

	local UTCTime5 = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "UTCTime5 : " .. UTCTime5)
	curr = curr + 1

	local Timestamp = buffer(curr, 4):uint()
	header_subtree:add(buffer(curr, 4), "Timestamp  : " .. Timestamp)
	curr = curr + 4

	local MeasureMode = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "MeasureMode : " .. MeasureMode)
	curr = curr + 1

	local LaserNum = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "LaserNum : " .. LaserNum)
	curr = curr + 1

	local BlockNum = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "BlockNum : " .. BlockNum)
	curr = curr + 1

	local EchoCount = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "EchoCount : " .. EchoCount)
	curr = curr + 1

	local TimeSyncMode = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "TimeSyncMode : " .. TimeSyncMode)
	curr = curr + 1

	local TimeSyncStat = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "TimeSyncStat : " .. TimeSyncStat)
	curr = curr + 1

	local MemsTemp = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "MemsTemp : " .. MemsTemp)
	curr = curr + 1

	local SlotNum = buffer(curr, 1):uint()
	header_subtree:add(buffer(curr, 1), "SlotNum : " .. SlotNum)
	curr = curr + 1

	local PointNum = buffer(curr, 2):uint()
	header_subtree:add(buffer(curr, 2), "PointNum : " .. PointNum)
	curr = curr + 2

	local Reserved1 = buffer(curr, 2):uint()
	header_subtree:add(buffer(curr, 2), "Reserved1 : " .. Reserved1)
	curr = curr + 2

	local Reserved2 = buffer(curr, 2):uint()
	header_subtree:add(buffer(curr, 2), "Reserved2 : " .. Reserved2)
	curr = curr + 2


	---- bock Return ----
	local size = blockPerPacket * blockSize
	local blockreturns = subtree:add(buffer(curr, size), "Blocks")

	for i=0, blockPerPacket-1
	do
		local block_subtree = blockreturns:add(buffer(curr,blockSize), "Block Return : " ..i)

		local channelNum = buffer(curr, 1):uint()
		block_subtree:add(buffer(curr, 1), "channelNum : " .. channelNum)
		curr = curr + 1

		local timeOffSet = buffer(curr, 1):uint()
		block_subtree:add(buffer(curr, 1), "timeOffSet : " .. timeOffSet)
		curr = curr + 1

		local returnSn = buffer(curr, 1):uint()
		block_subtree:add(buffer(curr, 1), "returnSn : " .. returnSn)
		curr = curr + 1

		local reserved = buffer(curr, 1):uint()
		block_subtree:add(buffer(curr, 1), "reserved : " .. reserved)
		curr = curr + 1
		
		local all_laser_subtree = block_subtree:add(buffer(curr, laserSize * nbLaser), "All Lasers Return")

		for j=0, nbLaser-1
		do
			local laser_subtree = all_laser_subtree:add(buffer(curr, laserSize), "Laser Return : " ..j)

			local Distance = buffer(curr, 2):uint()
			laser_subtree:add(buffer(curr, 2), "Distance  : " .. Distance)
			curr = curr + 2

			local Azimuth = buffer(curr, 2):uint()
			laser_subtree:add(buffer(curr, 2), "Azimuth  : " .. Azimuth)
			curr = curr + 2

			local Elevation = buffer(curr, 2):uint()
			laser_subtree:add(buffer(curr, 2), "Elevation  : " .. Elevation)
			curr = curr + 2

			local Intensity = buffer(curr, 1):uint()
			laser_subtree:add(buffer(curr, 1), "Intensity  : " .. Intensity)
			curr = curr + 1

			local Reserved = buffer(curr, 2):uint()
			laser_subtree:add(buffer(curr, 2), "Reserved  : " .. Reserved)
			curr = curr + 2
		end

	end


	-- Tail --
	local tail_subtree = subtree:add(buffer(curr, tailSize), "Tail")
	
	--[[
	local CRC = buffer(curr, 4):uint()
	tail_subtree:add(buffer(curr,4),"CRC : " .. CRC)
	curr = curr + 4

	for n=0, 16
	do
		local functionSafety_subtree = tail_subtree:add(buffer(curr,1),"functionSafety subtree : " ..n)

		local functionSafety = buffer(curr,1):uint()
		functionSafety_subtree:add(buffer(curr,1),"functionSafety  : " .. functionSafety)
		curr = curr + 1
	end
	--]]

	local TailFlag = buffer(curr, 4):uint()
	tail_subtree:add(buffer(curr, 4), "TailFlag : " .. TailFlag)
	curr = curr + 4

end


-- load the udp.port table
udp_table = DissectorTable.get("udp.port")
-- register our protocol to handle udp port 51180
udp_table:add(51180, lidar_proto)
