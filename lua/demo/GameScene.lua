require "Cocos2d"
require "Cocos2dConstants"
require "FerryConstants"

local GameScene = class("GameScene",function()
    return cc.Scene:create()
end)

function GameScene.create()
    local scene = GameScene.new()
    return scene
end

function GameScene:ctor()
    self:start_ferry()
end

function GameScene:start_ferry()
    ferry.ScriptFerry:getInstance():init("127.0.0.1", 7777)

    local scene = cc.Scene:create()

    local eventEntryID = ferry.ScriptFerry:getInstance():addEventCallback(
        function(event)
            print('event', event:getWhat(), event:getCode(), event:getBox())
            if (event:getWhat() == ferry.EventType.open) then
                local box = ferry.ScriptFerry:getInstance():createBox()
                box:setCmd(1)
                box:setBody("woaini")
                local rspEntryID = ferry.ScriptFerry:getInstance():send(box,
                    function(event)
                        print('rsp', event:getWhat(), event:getCode(), event:getBox())
                        if (event:getBox()) then
                            print("box", event:getBox():getCmd(), event:getBox():getRet(), event:getBox():getBody())
                        end

                        --[[
                        if event:getWhat() == ferry.EventType.recv then
                            ferry.ScriptFerry:getInstance():disconnect()
                            -- ferry.ScriptFerry:getInstance():stop()
                        end
                        ]]
                    end
                    , 0.5, self)

                -- ferry.ScriptFerry:getInstance():delRspCallback(rspEntryID)

            elseif (event:getWhat() == ferry.EventType.close) then
                ferry.ScriptFerry:getInstance():connect()
            end
        end, self)

    -- ferry.ScriptFerry:getInstance():delEventCallback(eventEntryID)
    -- ferry.ScriptFerry:getInstance():delEventCallbacksForTarget(self)

    ferry.ScriptFerry:getInstance():start()
end

return GameScene
