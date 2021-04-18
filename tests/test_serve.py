from entity_base import Swarm, Entity, EntityID

def test_serve_many_blocks_random_toplogy(nodes=20, blocks=50):
    with Swarm(nodes) as swarm:
        swarm.connect_randomly()
        swarm.sleep(0.1)
        bottom = EntityID(1)
        for n in range(1, blocks):
            swarm.nodes[0].put_entity(Entity(n, "block {}".format(n)))

        tries = nodes * blocks

        while tries > 0:
            for node in swarm.nodes[1:]:
                their_top = node.remote_top_id()
                assert their_top
                our_top = node.local_top_id()
                assert our_top
                if our_top < their_top:
                    node.fetch_remote_entity(our_top.increment())
            swarm.sleep(0.1)
            tries -= 1
        for n in range(1, blocks):
            for node in swarm.nodes:
                if not node.has_entity_at_index(n):
                    print("{} lacks block {}".format(node.addr(), n))
                    assert False
