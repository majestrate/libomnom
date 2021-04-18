import pyomnom

import os
import random
import time

class Storage(pyomnom.EntityStorage):

    def __init__(self, name):
        self._name = name
        pyomnom.EntityStorage.__init__(self)
        self._entities = dict()
        self._maxid = None
        
    def HasEntity(self, ent):
        return ent.ID.value() in self._entities

    def StoreEntity(self, ent):
        self._entities[ent.ID.value()] = ent
        if self._maxid is None or self._maxid < ent.ID:
            self._maxid = ent.ID
        
    def HasEntityByID(self, id):
        return id.value() in self._entities

    def GetTopEntityID(self):
        return self._maxid


    def GetEntityByID(self, entID):
        if entID.value() in self._entities:
            return self._entities[entID.value()]
        
    
    def IDs(self):
        ids = []
        for id in self._entities.keys():
            ids.append(id)
        return ids

class EntityID(pyomnom.EntityID):

    def __init__(self, idx):
        self._idx = idx
        pyomnom.EntityID.__init__(self, idx)

    def increment(self, amount=1):
        return EntityID(self._idx + amount)
        
        
    def __repr__(self):
        return "index={}".format(self._idx)

    def __lt__(self, other):
        return self._idx < other._idx
    
TestEntityKind = pyomnom.EntityKind("test-kind", False)


class Entity(pyomnom.Entity):

    def __init__(self, idx, data):
        self._idx = idx
        pyomnom.Entity.__init__(self)
        self.ID = EntityID(idx)
        self.Kind = TestEntityKind
        self.set_data(str(data))

    def __repr__(self):
        return "Entity: index={} kind={}".format(self._idx, self.Kind.name)

class Node:

    baseport = 25000

    def __init__(self, node_id):
        self._storage = Storage("node-{}".format(node_id))
        self._ctx = pyomnom.Context("test")
        self._addr = 'tcp://127.0.1.1:{}'.format(self.baseport + node_id)
        self.node_id = node_id
        self._ctx.set_storage_for(TestEntityKind, self._storage)
        self._ctx.add_entity_handler(TestEntityKind, self.handleGotEntity)
        self._ctx.listen(self._addr)

    def fetch_remote_entity(self, id):
        pass

    def local_top_id(self):
        return self._storage.GetTopEntityID()
        
    def put_entity(self, ent):
        self._storage.StoreEntity(ent)
        
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
        pyomnom.sleep_ms(int(num * 1000))

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
