import pyentsync

import os
import random
import time

class Storage(pyentsync.EntityStorage):

    def __init__(self):
        pyentsync.EntityStorage.__init__(self)
        self._entities = dict()
    
    def HasEntity(self, ent):
        return ent.ID in self._entities

    def StoreEntity(self, ent):
        self._entities[ent.ID] = ent

    def HasEntityByID(self, id):
        return id in self._entities
        
class Entity(pyentsync.Entity):

    def __init__(self, idx, data):
        pyentsync.Entity.__init__(self)
        self.ID = pyentsync.EntityID(idx)
        self.Kind = pyentsync.EntityKind("test-entity", False)
        self.set_data(str(data))
        
class Node:

    baseport = 25000

    def __init__(self, node_id):
        self._storage = Storage()
        self._ctx = pyentsync.Context("test")
        self._addr = 'tcp://127.0.1.1:{}'.format(self.baseport + node_id)
        self.node_id = node_id
        self._ctx.listen(self._addr)

    def has_entity_at_index(self, idx):
        return self._storage.HasEntityByID(pyentsync.EntityID(idx))
         
    def addr(self):
        return self._addr
        
    def start(self):
        self._ctx.start()

    def broadcast(self, ent):
        self._ctx.broadcast_entity(ent)

    def add_peer(self, addr):
        self._ctx.add_peer(addr)

    def peers(self):
        return self._ctx.peers()


def makeNetGraph(nodes):
    G = nx.Graph()
    for node in nodes:
        G.add_node(node.addr())
        for peer in node.peers():
            edge = node.addr(), peer
            if edge not in G.edges:
                G.add_edge(*edge)
    return G


def makeNetwork(numNodes=10):
    nodes = []
    for n in range(numNodes):
        ctx = Node(n)
        ctx.start()
        nodes.append(ctx)
    return nodes

def randomlyConnect(nodes):
    alice = random.choice(nodes)
    bob = random.choice(nodes)
    if alice.addr() != bob.addr():
        alice.add_peer(bob.addr())

def connect_sequentially(nodes):
    """
    given a list of nodes connect 0 to 1, 1 to 2 ... N-1 to N
    """
    for idx in range(len(nodes)):
        if idx < len(nodes) - 1:
            nodes[idx].add_peer(nodes[idx+1].addr())
            
    
def test_broadcast():
    nodes = makeNetwork()
    connect_sequentially(nodes)
    time.sleep(1)
    ent = Entity(1, "test data")
    nodes[1].broadcast(ent)
    time.sleep(1)
    #for node in nodes:
    #    assert node.has_entity_at_index(1)
    
def runsim(numNodes=30):
    nodes = makeNetwork(numNodes)

    fig, ax = plt.subplots()

    def animate(i):
        if i % 5 == 1:
            randomlyConnect(nodes)
        fig.clear()
        G = makeNetGraph(nodes)
        if len(G.edges) == 0:
            return
        #pos = nx.kamada_kawai_layout(G)
        pos = nx.circular_layout(G)
        nx.draw_networkx_nodes(G, pos, node_size=700)
        nx.draw_networkx_edges(G, pos)
        nx.draw_networkx_labels(G, pos)
    
    ani = animation.FuncAnimation(fig, animate, interval=100)
    ax = plt.gca()
    ax.margins(0.08)
    plt.axis("off")
    plt.tight_layout()
    plt.show()
        
if __name__ == '__main__':
    import networkx as nx
    import matplotlib.pyplot as plt
    import matplotlib.animation as animation
    runsim(50)
