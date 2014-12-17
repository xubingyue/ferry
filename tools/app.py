from gevent import monkey; monkey.patch_all()
from haven import THaven
from netkit.box import Box

import logging

logger = logging.getLogger('haven')
logger.addHandler(logging.StreamHandler())
logger.setLevel(logging.DEBUG)

app = THaven(Box)

@app.route(1)
def index(request):
    print '\n', 'req', request.box
    request.box.ret = 100
    request.box.body = request.box.body[::-1]

    print 'rsp', request.box, '\n'
    request.write(request.box)

    box = Box()
    box.cmd = 15
    print 'event', box, '\n'
    request.write(box)

app.run(host="0.0.0.0", port=7777, debug=True)
