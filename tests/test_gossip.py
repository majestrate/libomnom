import pyentsync

import os
import random
import time

class Storage(pyentsync.EntityStorage):

    def __init__(self, name):
        self._name = name
        pyentsync.EntityStorage.__init__(self)
        self._entities = dict()

    def HasEntity(self, ent):
        print("check if {} has {}".format(self._name, ent))
        return ent.ID.value() in self._entities

    def StoreEntity(self, ent):
        print("store entity {} in {} at {}".format(ent, self._name, ent.ID))
        self._entities[ent.ID.value()] = ent

    def HasEntityByID(self, id):
        print("check if {} has at {}".format(self._name, id))
        return id.value() in self._entities

    def IDs(self):
        ids = []
        for id in self._entities.keys():
            ids.append(id)
        return ids

class EntityID(pyentsync.EntityID):

    def __init__(self, idx):
        self._idx = idx
        pyentsync.EntityID.__init__(self, idx)


    def __repr__(self):
        return "index={}".format(self._idx)

TestEntityKind = pyentsync.EntityKind("test-kind", False)


class Entity(pyentsync.Entity):

    def __init__(self, idx, data):
        self._idx = idx
        pyentsync.Entity.__init__(self)
        self.ID = EntityID(idx)
        self.Kind = TestEntityKind
        self.set_data(str(data))

    def __repr__(self):
        return "Entity: index={} kind={}".format(self._idx, self.Kind.name)

class Node:

    baseport = 25000

    def __init__(self, node_id):
        self._storage = Storage("node-{}".format(node_id))
        self._ctx = pyentsync.Context("test")
        self._addr = 'tcp://127.0.1.1:{}'.format(self.baseport + node_id)
        self.node_id = node_id
        self._ctx.set_storage_for(TestEntityKind, self._storage)
        self._ctx.add_entity_handler(TestEntityKind, self.handleGotEntity)
        self._ctx.listen(self._addr)

    def stop(self):
        del self._ctx
        self._ctx = None

    def handleGotEntity(self, ent):
        print("{} got {}".format(self.addr(), ent))
        if self._storage.HasEntity(ent):
            print("dropping {}".format(ent))
            return
        else:
            self._storage.StoreEntity(ent)
        print("repeating {}".format(ent))
        self.broadcast(ent)

    def has_entity_at_index(self, idx):
        return self._storage.HasEntityByID(EntityID(idx))

    def getAllIDsKnown(self):
        return self._storage.IDs()

    def getMaxID(self):
        maxid = ''
        for id in self._storage.IDs():
            if id > maxid:
                maxid = id
        return maxid

    def addr(self):
        return self._addr

    def start(self):
        self._ctx.start()

    def broadcast(self, ent):
        self._storage.StoreEntity(ent)
        print("broadcasting: {}".format(ent))
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

def connect_random(nodes):
    copy_nodes = list(nodes)
    random.shuffle(copy_nodes)
    connect_sequentially(copy_nodes)

def test_broadcast_seq_toplogy():
    nodes = makeNetwork(50)
    connect_sequentially(nodes)
    time.sleep(1)
    ent = Entity(1, "test data")
    nodes[0].broadcast(ent)
    time.sleep(1)
    for node in nodes:
        print("test node {}".format(node.addr()))
        assert node.has_entity_at_index(1)
        node.stop()

def test_broadcast_random_topology():
    nodes = makeNetwork(50)
    time.sleep(1)
    connect_random(nodes)
    time.sleep(1)
    ent = Entity(1, "test data")
    nodes[0].broadcast(ent)
    time.sleep(1)
    for node in nodes:
        print("test node {}".format(node.addr()))
        assert node.has_entity_at_index(1)
        node.stop()

def makeLabels(G, nodes):
    labels = dict()
    addrs = dict()
    for node in nodes:
        addrs[node.addr()] = node
    for edge in G.nodes:
        labels[edge] = '{} with {}'.format(edge, addrs[edge].getAllIDsKnown())
    return labels


def runsim(numNodes):
    nodes = makeNetwork(numNodes)
    fig, ax = plt.subplots()
    def animate(i):
        step = i % 10
        if step == 1:
            connect_random(nodes)
        if step > 0:
            ent = Entity(i, "test data")
            print('gossiping {}'.format(ent))
            nodes[0].broadcast(ent)

        fig.clear()
        G = makeNetGraph(nodes)
        if len(G.edges) == 0:
            return
        #pos = nx.kamada_kawai_layout(G)
        pos = nx.circular_layout(G)
        nx.draw_networkx_nodes(G, pos, node_size=700)
        nx.draw_networkx_edges(G, pos)
        nx.draw_networkx_labels(G, pos, makeLabels(G, nodes))

    ani = animation.FuncAnimation(fig, animate, interval=500)
    ax = plt.gca()
    ax.margins(0.08)
    plt.axis("off")
    plt.tight_layout()
    plt.show()

if __name__ == '__main__':
    import networkx as nx
    import matplotlib.pyplot as plt
    import matplotlib.animation as animation
    runsim(5)
