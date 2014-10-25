ferry = ferry or {}

ferry.EventType = {
    open = 1,
    send = 2,
    recv = 3,
    close = 4,
    error = 5,
    timeout = 6,
}

ferry.ErrorCode = {
    open = 1,
    send = 2,
}
