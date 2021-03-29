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
        return ent.ID.value() in self._entities

    def StoreEntity(self, ent):
        self._entities[ent.ID.value()] = ent

    def HasEntityByID(self, id):
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

    def handleGotEntity(self, ent, peer):
        if self._storage.HasEntity(ent):
            return
        else:
            self._storage.StoreEntity(ent)
        self.broadcast(ent, lambda other : other.uid() != peer.uid())

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

    def broadcast(self, ent, peer_filter=None):
        self._storage.StoreEntity(ent)
        if peer_filter is None:
            peer_filter = lambda x: True
        self._ctx.broadcast_entity(ent, peer_filter)


    def add_peer(self, addr):
        self._ctx.add_peer(addr)

    def peers(self):
        return self._ctx.peers()


def makeNetwork(numNodes=10):
    nodes = []
    for n in range(numNodes):
        ctx = Node(n)
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
    nodes = None

def connect_random(nodes):
    copy_nodes = list(nodes)
    random.shuffle(copy_nodes)
    connect_sequentially(copy_nodes)
    copy_nodes = None


class Swarm:

    def __init__(self, num):
        self.nodes = makeNetwork(num)

    def sleep(self, num):
        pyentsync.sleep_ms(int(num * 1000))

    def connect_randomly(self):
        connect_random(self.nodes)

    def connect_sequentially(self):
        connect_sequentially(self.nodes)

    def __enter__(self, *args):
        for node in self.nodes:
            node.start()
        return self

    def __exit__(self, *args):
        for node in self.nodes:
            node.stop()
        del self.nodes
