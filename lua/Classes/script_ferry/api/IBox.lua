
--------------------------------
-- @module IBox
-- @parent_module ferry

--------------------------------
-- @function [parent=#IBox] packetLen 
-- @param self
-- @return int#int ret (return value: int)
        
--------------------------------
-- @function [parent=#IBox] toString 
-- @param self
-- @return string#string ret (return value: string)
        
--------------------------------
-- @function [parent=#IBox] unpack 
-- @param self
-- @param #char char
-- @param #int int
-- @return int#int ret (return value: int)
        
--------------------------------
-- @function [parent=#IBox] check 
-- @param self
-- @param #char char
-- @param #int int
-- @return int#int ret (return value: int)
        
--------------------------------
-- @overload self, string         
-- @overload self, char, int         
-- @function [parent=#IBox] pack
-- @param self
-- @param #char char
-- @param #int int
-- @return int#int ret (retunr value: int)

return nil
