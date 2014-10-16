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
    print request.box
    request.box.ret = 100
    request.write(request.box)

app.run(port=7777, debug=True)
