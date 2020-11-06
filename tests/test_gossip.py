import pyentsync

import os
import time


class Node:

    def __init__(self, node_id):
        self._ctx = pyentsync.Context("test")
        self._sockfile = 'test-{}.sock'.format(node_id)
        self._ctx.listen('ipc://{}'.format(self._sockfile))

    def addr(self):
        return self._sockfile
        
    def start(self):
        self._ctx.start()


    def add_peer(self, addr):
        self._ctx.add_peer(addr)
        
    def __del__(self):
        del self._ctx
        if os.path.exists(self._sockfile):
            os.unlink(self._sockfile)

        


def test_fullmesh():
    nodes = []
    for n in range(30):
        ctx = Node(n)
        ctx.start()
        nodes.append(ctx)

    for n in range(1, 30):
        nodes[n].add_peer(nodes[n - 1].addr())
        time.sleep(0.5)
        
if __name__ == '__main__':
    test_fullmesh()
