import pyentsync

import os
import time


class Node:

    baseport = 25000

    def __init__(self, node_id):
        self._ctx = pyentsync.Context("test")
        self._addr = 'tcp://127.0.1.1:{}'.format(self.baseport + node_id)
        self._ctx.listen(self._addr)

    def addr(self):
        return self._addr
        
    def start(self):
        self._ctx.start()


    def add_peer(self, addr):
        self._ctx.add_peer(addr)

        


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
