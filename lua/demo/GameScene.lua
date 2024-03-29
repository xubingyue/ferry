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
    local layer = cc.LayerColor:create(cc.c4b(0,0,255,255))
    self:addChild(layer)

    self:regHandlers()

    self:startFerry()
end

function GameScene:regHandlers()
    self:registerScriptHandler(
        function (event_type)
            print(self.__cname, "event_type:", event_type, type(event_type))

            if (event_type == "enter") then
                if self.onEnter then
                    self:onEnter()
                end
            elseif (event_type == "enterTransitionFinish") then
                if self.onEnterTransitionFinish then
                    self:onEnterTransitionFinish()
                end
            elseif (event_type == "exitTransitionStart") then
                if self.onExitTransitionStart then
                    self:onExitTransitionStart()
                end
            elseif (event_type == "exit") then
                if self.onExit then
                    self:onExit()
                end
            elseif (event_type == "cleanup") then
                if self.onCleanup then
                    self:onCleanup()
                end
            end
        end
    )

end

function GameScene:onCleanup()
    ferry.ScriptFerry:getInstance():delCallbacksForTarget(self)
end

function GameScene:startFerry()
    ferry.ScriptFerry:getInstance():init("127.0.0.1", 7777)

    local scene = cc.Scene:create()

    local eventEntryID = ferry.ScriptFerry:getInstance():addEventCallback(
        function(event)
            print('event', event:getWhat(), event:getCode(), event:getBox())
            if (event:getWhat() == ferry.EventType.open) then
                local box = ferry.ScriptFerry:getInstance():createBox()
                box:setCmd(1)
                -- 二进制也可以
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
                -- ferry.ScriptFerry:getInstance():delRspCallbacksForTarget(self)

            elseif (event:getWhat() == ferry.EventType.close) then
                ferry.ScriptFerry:getInstance():connect()
            elseif (event:getWhat() == ferry.EventType.error) then
                -- 连接失败
            elseif (event:getWhat() == ferry.EventType.timeout) then
                -- 连接超时
            end
        end, self)

    -- ferry.ScriptFerry:getInstance():delEventCallback(eventEntryID)
    -- ferry.ScriptFerry:getInstance():delEventCallbacksForTarget(self)
    
    -- ferry.ScriptFerry:getInstance():delCallbacksForTarget(self)

    ferry.ScriptFerry:getInstance():start()
end

return GameScene
