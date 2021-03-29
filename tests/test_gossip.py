from entity_base import Swarm, Entity

import time

def test_broadcast_one_seq_toplogy(nodes=20):
    with Swarm(nodes) as swarm:
        swarm.connect_sequentially()
        swarm.sleep(0.1)
        ent = Entity(1, "test data")
        swarm.nodes[0].broadcast(ent)
        swarm.sleep(0.1)
        for node in swarm.nodes:
            assert node.has_entity_at_index(1)

def test_broadcast_many_random_topology_many_peers(nodes=20, ents=10, peers=5):
    with Swarm(nodes) as swarm:
        for _ in range(peers):
            swarm.connect_randomly()

        swarm.sleep(1)

        for num in range(1, ents):
            ent = Entity(num, "test data {}".format(num))
            swarm.nodes[0].broadcast(ent)

        swarm.sleep(0.01 * ents)

        fail = False
        for node in swarm.nodes:
            for num in range(1, ents):
                if not node.has_entity_at_index(num):
                    print("{} does not have {}".format(node.addr(), num))
                    fail = True
        assert not fail

test_broadcast_one_random_topology_single_peer = lambda : test_broadcast_many_random_topology_many_peers(20, 10, 1)
test_shit_tons_of_sync = lambda : test_broadcast_many_random_topology_many_peers(20, 1000, 5)
