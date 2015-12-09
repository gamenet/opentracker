local logtable = {}

local function logit(msg)
    logtable[#logtable+1] = msg
end

local function split(splitPattern, sourceString)
    local res = {}
    local currentIndex = 0

    while true do
        local l = string.find(sourceString, splitPattern, currentIndex, true)
        if l~=nil then
            table.insert(res, string.sub(sourceString, currentIndex, l - 1))
            currentIndex = l + 1
        else
            table.insert(res, string.sub(sourceString, currentIndex))
            break
        end
    end
    return res
end

local function hGetAll(key)
    local rawData = redis.call('HGETALL', key)
    local hashData = {}

    for idx = 1, #rawData, 2 do
        hashData[rawData[idx]] = rawData[idx + 1]
    end

    return hashData;
end

local function loadUserData(userId)
    local userData = hGetAll(userId)
    local events = {}

    for field, value in pairs(userData) do
        local parts = split(".", field);

        local infoHash = parts[1];
        local sessionId = parts[2];
        local name = parts[3];

        if events[infoHash] == nil then
            events[infoHash] = {};
        end

        if events[infoHash][sessionId] == nil then
            events[infoHash][sessionId] = {}
        end

        events[infoHash][sessionId][name] = value;
    end

    return events;
end

-------------------------------------------------------------------------------
----------                  Application code                         ----------
-------------------------------------------------------------------------------

local lastUpdated = redis.call('smembers', 'lastUpdated')
local totalUpdatedUsers = 0
local totalUpdatedRecords = 0

for index, userId in pairs(lastUpdated) do
    totalUpdatedUsers = totalUpdatedUsers + 1

    local events = loadUserData(userId);

    for hashInfo in pairs(events) do
        local gameId = redis.call('hget', 'hashinfo', string.lower(hashInfo));
        if gameId ~= false then
            for sessionId in pairs(events[hashInfo]) do
                local status = events[hashInfo][sessionId]["status"]
                if status == "stopped" then
                    local uploaded = events[hashInfo][sessionId]["uploaded"]
                    local downloaded = events[hashInfo][sessionId]["downloaded"]
                    local totalKey = "total:" .. userId

                    redis.call("zincrby", "rating:downloaded", downloaded, userId);
                    redis.call("zincrby", "rating:uploaded", uploaded, userId);
                    redis.call("zincrby", "rating:downloaded:" .. gameId, downloaded, userId);
                    redis.call("zincrby", "rating:uploaded:" .. gameId, uploaded, userId);
                    redis.call("hincrby", totalKey, "uploaded", uploaded);
                    redis.call("hincrby", totalKey, "downloaded", downloaded);
                    redis.call("hincrby", totalKey, "uploaded:" .. gameId, uploaded);
                    redis.call("hincrby", totalKey, "downloaded:" .. gameId, downloaded);

                    totalUpdatedRecords = totalUpdatedRecords + 1
                end
            end
        else
            redis.log(redis.LOG_NOTICE, "Unknown info hash `" .. hashInfo .. "`")
        end
    end

    redis.call("del", userId)
    redis.call("srem", "lastUpdated", userId)
end

logit("Updated users: " .. totalUpdatedUsers)
logit("Updated records: " .. totalUpdatedRecords)

return logtable