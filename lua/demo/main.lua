function main()
    require "FerryConstants"

    ferry.ScriptFerry:getInstance():init("127.0.0.1", 7777)
    ferry.ScriptFerry:getInstance():start()
    print("ferry.EventType.open", ferry.EventType.open)

    ferry.ScriptFerry:getInstance():addEventCallback(
        function(event)
            print('event', event:getWhat(), event:getCode(), event:getBox())
            if (event:getWhat() == ferry.EventType.open) then
                local box = ferry.ScriptFerry:getInstance():createBox()
                box:setCmd(1)
                box:setBody("woaini")
                ferry.ScriptFerry:getInstance():send(box,
                    function(event)
                        print('rsp', event:getWhat(), event:getCode(), event:getBox())
                        if (event:getBox()) then
                            print("box", event:getBox():getCmd(), event:getBox():getRet(), event:getBox():getBody())
                        end
                    end
                    , 5)
            end
        end
        )
end
