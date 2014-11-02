
--------------------------------
-- @module Box
-- @extend IBox
-- @parent_module ferry

--------------------------------
-- @function [parent=#Box] bodyLen 
-- @param self
-- @return int#int ret (return value: int)
        
--------------------------------
-- @function [parent=#Box] packetLen 
-- @param self
-- @return int#int ret (return value: int)
        
--------------------------------
-- @function [parent=#Box] headerLen 
-- @param self
-- @return int#int ret (return value: int)
        
--------------------------------
-- @function [parent=#Box] toString 
-- @param self
-- @return string#string ret (return value: string)
        
--------------------------------
-- @function [parent=#Box] unpackDone 
-- @param self
-- @return bool#bool ret (return value: bool)
        
--------------------------------
-- @overload self, string         
-- @overload self, char, int         
-- @function [parent=#Box] pack
-- @param self
-- @param #char char
-- @param #int int
-- @return int#int ret (retunr value: int)

--------------------------------
-- @function [parent=#Box] unpack 
-- @param self
-- @param #char char
-- @param #int int
-- @return int#int ret (return value: int)
        
--------------------------------
-- @function [parent=#Box] check 
-- @param self
-- @param #char char
-- @param #int int
-- @return int#int ret (return value: int)
        
--------------------------------
-- @function [parent=#Box] Box 
-- @param self
        
return nil
