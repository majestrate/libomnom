from entity_base import Swarm, Entity

import time

def test_broadcast_seq_toplogy():
    with Swarm(50) as swarm:
        swarm.connect_sequentially()
        time.sleep(1)
        ent = Entity(1, "test data")
        swarm.nodes[0].broadcast(ent)
        time.sleep(1)
        for node in swarm.nodes:
            print("test node {}".format(node.addr()))
            assert node.has_entity_at_index(1)

def test_broadcast_random_topology_single_peer():
    with Swarm(50) as swarm:
        swarm.connect_randomly()
        time.sleep(1)
        ent = Entity(1, "test data")
        swarm.nodes[0].broadcast(ent)
        time.sleep(1)
        for node in swarm.nodes:
            print("test node {}".format(node.addr()))
            assert node.has_entity_at_index(1)

