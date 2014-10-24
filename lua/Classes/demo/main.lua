require("FerryConstants")

function main()
    ferry.ScriptFerry:getInstance():init("127.0.0.1", 7777)
    ferry.ScriptFerry:getInstance():start()

    ferry.ScriptFerry:getInstance():addEventCallback(function(event)
        print('rsp', event:getWhat(), event:getCode(), event:getBox())

        local box = ferry.ScriptFerry:getInstance():createBox()
        box:setCmd(1)
        box:setBody("woaini")
        ferry.ScriptFerry:getInstance():send(box, function(event)
            print('rsp', event:getWhat(), event:getCode(), event:getBox())
            if (event:getBox()) then
                print("box", event:getBox():getCmd(), event:getBox():getRet(), event:getBox():getBody())
            end
        end, 5)
    end) 
end

main()
